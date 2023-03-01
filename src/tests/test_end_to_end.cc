#include <chrono>
#include <iostream>
#include <cmath>
#include <random>
#include <thread>
#include "model.hh"
#include "timing_logger.hh"
#include "fec_datagram.hh"
#include "multi_fec_header_code.hh"
#include "streaming_code.hh"
#include "simple_quality_report_generator.hh"
#include "reed_solomon_multi_frame_packetization.hh"
#include "streaming_code.hh"
#include "multi_fec_header_code.hh"
#include "frame_generator.hh"
#include "frame.hh"
#include "fec_sender.hh"
#include "fec_multi_receiver.hh"
#include "frame_logger.hh"
#include "streaming_code_packetization.hh"
#include "quality_reporter.hh"
#include "input_converter.hh"
#include "loss_info.hh"
#include "helpers.hh"

using namespace std;

bool test_simple_encode_decode_no_loss()
{
  srand(0);
  for (uint16_t trial = 0; trial < 20; trial++) {
    for (uint16_t tau = 0; tau < 4; tau++) {
      for (uint16_t size_factor = 1; size_factor < 2; size_factor++) {
        uint16_t seed = 0;
        uint16_t w = 32;
        uint16_t max_frame_size_possible = 10000; 
        uint16_t packet_size = 8 * size_factor;
        uint16_t stripe_size = packet_size * w;
        uint16_t max_data_frame_pkts = 100;
        uint16_t max_fec_frame_pkts = 75;
        uint16_t max_data_stripes_per_frame = 100;
        uint16_t max_fec_stripes_per_frame = 75 ;
        assert (max_data_stripes_per_frame * stripe_size >= max_frame_size_possible);
        assert (max_fec_stripes_per_frame * stripe_size >= max_frame_size_possible / 2);
        assert (pow(2, w) >= uint64_t(num_frames_for_delay(tau)) * uint64_t(num_frames_for_delay(tau))
            * max_data_stripes_per_frame * max_fec_stripes_per_frame);
        uint16_t num_frames = num_frames_for_delay(tau);
        uint16_t n_cols = num_frames;
        uint16_t n_rows = 256;
        while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
        CodingMatrixInfo codingMatrixInfoHeader { n_rows, n_cols, 8};

        CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(tau) *
            max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * max_data_stripes_per_frame), w};
        for (uint16_t rs_packetization = 0; rs_packetization < 2; rs_packetization++) {
          Packetization * packetizationSender;
          if (rs_packetization == 0) {
            packetizationSender = new StreamingCodePacketization(w, stripe_size, map<int, pair<int, int>> { { 0 , { 0, 1} }, { 1, { 1, 2} } }, uint16_t(Packetization::MTU));
          }
          else {
            packetizationSender = new ReedSolomonMultiFramePacketization(stripe_size, map<int, pair<int, int>> { { 0 , { 0, 1} }, { 1, { 1, 2} } }, tau + 1, w,
                uint16_t(Packetization::MTU));
          }

          BlockCode blockCodeHeaderSender(codingMatrixInfoHeader, tau,
                                          pair<uint16_t, uint16_t>{1, 0}, 8, false);
          MultiFECHeaderCode headerCodeSender(&blockCodeHeaderSender, tau,
              codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));
          BlockCode blockCodeFECSender(codingMatrixInfoFEC, tau,
                                       pair<uint16_t, uint16_t>{1, rs_packetization == 0}, packet_size, false);
          StreamingCode codeSender(tau, stripe_size,  &blockCodeFECSender, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);

          std::vector<MetricLogger *> metricLoggersSender;
          Logger loggerSender("tests/", metricLoggersSender);
          FrameGenerator frameGenerator(&codeSender, tau, packetizationSender,
              &headerCodeSender, &loggerSender, seed);
          FECSender fECSender(frameGenerator, tau, &loggerSender, 1);
          FeedbackDatagram feedback {1};
          fECSender.handle_feedback(feedback.serialize_to_string());
          BlockCode blockCodeHeaderReceiver(codingMatrixInfoHeader, tau,
                                            pair<uint16_t, uint16_t>{1, 0}, 8, false);
          MultiFECHeaderCode headerCodeReceiver(&blockCodeHeaderReceiver, tau,
              codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));
          BlockCode blockCodeFECReceiver(codingMatrixInfoFEC, tau,
                                         pair<uint16_t, uint16_t>{1, rs_packetization == 0}, packet_size, false);
          StreamingCode codeReceiver(tau, stripe_size,   &blockCodeFECReceiver, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);

          vector<MetricLogger *> metricLoggersReceiver;
          TimingLogger timingLoggerReceiver("test_log_receiver/");
          FrameLogger frameLoggerReceiver("test_log_receiver/");
          metricLoggersReceiver.push_back(&timingLoggerReceiver);
          Logger loggerReceiver("test_log_receiver/", metricLoggersReceiver, &frameLoggerReceiver);
          InputConverter * inputConverter = nullptr;
          Model * model = nullptr;
          QualityReportGenerator * simpleQualityReportGenerator;
          QualityReporter qualityReporter(model, inputConverter, simpleQualityReportGenerator);
          FECMultiReceiver fECMultiReceiver(&codeReceiver, tau, num_frames_for_delay(tau), &headerCodeReceiver, &qualityReporter,
              &loggerReceiver, 1);
          vector<string> frames;
          vector<uint16_t> frame_sizes;
          for (uint16_t frame_num = 0; frame_num < 5 * (tau + 1); frame_num++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            frames.push_back("");
            uint16_t frame_size = 3000 + 200 * frame_num  + (rand() % 2000);
            if ((trial % 2) == 0) {
              frame_size = (frame_size / stripe_size) * stripe_size;
            }
            frame_sizes.push_back(frame_size);
            auto pkts = fECSender.next_frame(frame_size);
            for (auto & pkt : pkts) {
              auto recv = pkt.serialize_to_string();
              if (!pkt.is_parity) { frames.at(frame_num).append(pkt.payload); }
              bool is_parity = pkt.is_parity;
              if (pkt.pos_in_frame > 0 and is_parity) {
                auto is_recovered = fECMultiReceiver.is_frame_recovered(frame_num);
                if (is_parity and !is_recovered.has_value()) {
                  assert(false);
                }
                if (is_parity and !is_recovered.value()) {
                  assert(false);
                }
                else if (is_parity) {
                  const auto recovery = fECMultiReceiver.recovered_frame(frame_num);
                  if (recovery.size() != frame_size) {
                    assert(false);
                  }
                  auto frame_val = frames.at(frame_num);
                  for (uint16_t pos = 0; pos < frame_size; pos++) {
                    if (recovery.at(pos) != frame_val.at(pos)) {
                      assert(false);
                    }
                  }
                }
              }
              fECMultiReceiver.receive_pkt(recv);
            }
          }
          for (uint16_t j = 0; j < num_frames_for_delay(tau); j++)
          {
            uint16_t frame_size = 3000 + 200 * j  + (rand() % 2000);
            if ((trial % 2) == 0) {
              frame_size = (frame_size / stripe_size) * stripe_size;
            }
            frame_sizes.push_back(frame_size);
            auto pkts = fECSender.next_frame(frame_size);
            for (auto & pkt : pkts) {
              auto recv = pkt.serialize_to_string();
              fECMultiReceiver.receive_pkt(recv);
            }
          }
          auto frame_logs = loggerReceiver.get_frame_logs();
          for (auto log : frame_logs) {
            if (log.not_decoded_) { assert(false); }
            assert(log.all_data_received_);
            if (log.decoded_after_frames_ != 0) { assert(false); }
          }
        }
      }
    }
  }
  return true;
}

bool test_loss_single_frame()
{
  srand(0);
  for (uint16_t size_factor = 1; size_factor < 3; size_factor++) {
    for (uint16_t trial = 0; trial < 20; trial++) {
      for (uint16_t tau = 0; tau < 4; tau++) {
        for (uint16_t rs_packetization = 0; rs_packetization < 2; rs_packetization++) {
          uint16_t seed = 0;
          uint16_t w = 32;
          uint16_t max_frame_size_possible = 12500; 
          uint16_t packet_size = 8 * size_factor;
          uint16_t stripe_size = packet_size * w;
          uint16_t max_data_frame_pkts = 64;
          uint16_t max_fec_frame_pkts = 32;
          uint16_t max_data_stripes_per_frame = 64;
          uint16_t max_fec_stripes_per_frame = 32;
          if ((tau > 1) and rs_packetization) {
            max_fec_stripes_per_frame *= tau;
            max_fec_frame_pkts *= tau;
          }
          assert (max_data_stripes_per_frame * stripe_size >= max_frame_size_possible);
          assert (max_fec_stripes_per_frame * stripe_size >= max_frame_size_possible / 2);
          assert (pow(2, w) >= uint64_t(num_frames_for_delay(tau)) * uint64_t(num_frames_for_delay(tau))
              * max_data_stripes_per_frame * max_fec_stripes_per_frame);
          uint16_t num_frames = num_frames_for_delay(tau);
          uint16_t n_cols = num_frames;
          uint16_t n_rows = 256;
          while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
          CodingMatrixInfo codingMatrixInfoHeader { n_rows, n_cols, 8};
          CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(tau) *
              max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * max_data_stripes_per_frame), w};
          Packetization * packetizationSender;
          if (rs_packetization == 0) {
            packetizationSender = new StreamingCodePacketization(w, stripe_size, map<int, pair<int, int>> { { 0 , { 0, 1} }, { 1, { 1, 2} } }, uint16_t(Packetization::MTU));
          }
          else {
            packetizationSender = new ReedSolomonMultiFramePacketization(stripe_size, map<int, pair<int, int>> { { 0 , { 0, 1} }, { 1, { 1, 2} } }, tau + 1, w, uint16_t(Packetization::MTU));
          }

          BlockCode blockCodeHeaderSender(codingMatrixInfoHeader, tau,
                                          pair<uint16_t, uint16_t>{1, 0}, 8, false);
          MultiFECHeaderCode headerCodeSender(&blockCodeHeaderSender, tau,
              codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));
          BlockCode blockCodeFECSender(codingMatrixInfoFEC, tau,
                                       pair<uint16_t, uint16_t>{1, rs_packetization == 0}, packet_size, false);
          StreamingCode codeSender(tau, stripe_size,  &blockCodeFECSender, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);

          std::vector<MetricLogger *> metricLoggersSender;
          Logger loggerSender("tests/", metricLoggersSender);
          FrameGenerator frameGenerator(&codeSender, tau, packetizationSender,
              &headerCodeSender, &loggerSender, seed);
          FECSender fECSender(frameGenerator, tau, &loggerSender, 1);
          FeedbackDatagram feedback {1};
          fECSender.handle_feedback(feedback.serialize_to_string());
          BlockCode blockCodeHeaderReceiver(codingMatrixInfoHeader, tau,
                                            pair<uint16_t, uint16_t>{1, 0}, 8, false);
          MultiFECHeaderCode headerCodeReceiver(&blockCodeHeaderReceiver, tau,
              codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));
          BlockCode blockCodeFECReceiver(codingMatrixInfoFEC, tau,
                                         pair<uint16_t, uint16_t>{1, rs_packetization == 0}, packet_size, false);
          StreamingCode codeReceiver(tau, stripe_size,  &blockCodeFECReceiver, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);

          vector<MetricLogger *> metricLoggersReceiver;
          TimingLogger timingLoggerReceiver("test_log_receiver/");
          FrameLogger frameLoggerReceiver("test_log_receiver/");
          metricLoggersReceiver.push_back(&timingLoggerReceiver);
          Logger loggerReceiver("test_log_receiver/", metricLoggersReceiver, &frameLoggerReceiver);
          InputConverter * inputConverter = nullptr;
          Model * model = nullptr;
          QualityReportGenerator * simpleQualityReportGenerator;
          QualityReporter qualityReporter(model, inputConverter, simpleQualityReportGenerator);
          FECMultiReceiver fECMultiReceiver(&codeReceiver, tau, num_frames_for_delay(tau), &headerCodeReceiver, &qualityReporter,
              &loggerReceiver, 1);
          vector<string> frames;
          vector<uint16_t> frame_sizes;
          vector<bool> data_lost;
          for (uint16_t frame_num = 0; frame_num < 7 * (tau + 1) + 1; frame_num++) {
            frames.push_back("");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            uint16_t frame_size = 3000 + (rand() % 2000);
            if ((trial % 2) == 0) {
              frame_size = (frame_size / stripe_size) * stripe_size;
            }
            frame_sizes.push_back(frame_size);
            auto pkts = fECSender.next_frame(frame_size);
            uint16_t missing_pkt = rand() % pkts.size();
            uint16_t highest_pos = pkts.size() -1 - (missing_pkt == pkts.size());
            data_lost.push_back(!pkts.at(missing_pkt).is_parity);
            for (const auto pkt : pkts) {
              auto recv = pkt.serialize_to_string();
              if (!pkt.is_parity) { frames.at(frame_num).append(pkt.payload); }
              if ((pkt.pos_in_frame == missing_pkt) and (frame_num > 2*tau + 1)) { continue; }
              fECMultiReceiver.receive_pkt(recv);
              if ((pkt.pos_in_frame == highest_pos) and (frame_num > 2*tau + 1)
                  and (!rs_packetization or ((frame_num % (tau + 1)) == tau))) {
                for (uint16_t check_frame = frame_num >= tau? frame_num - tau : 0; check_frame <= frame_num; check_frame++) {
                  auto is_recovered = fECMultiReceiver.is_frame_recovered(check_frame);
                  assert(is_recovered.has_value());
                  if(!fECMultiReceiver.is_frame_recovered(check_frame).value()) {
                    assert(false);
                  }
                  const auto recovery = fECMultiReceiver.recovered_frame(check_frame);
                  assert(recovery.size() == frame_sizes.at(check_frame));
                  auto frame_val = frames.at(check_frame);
                  for (uint16_t pos = 0; pos < frame_sizes.at(check_frame); pos++) {

                    if (recovery.at(pos) != frame_val.at(pos)) {
                      assert(false);
                    }
                  }
                }
              }
            }
          }
          for (uint16_t j = 0; j < num_frames_for_delay(tau); j++)
          {
            uint16_t frame_size = 3000 + 200 * j;
            if ((trial % 2) == 0) {
              frame_size = (frame_size / stripe_size) * stripe_size;
            }
            auto pkts = fECSender.next_frame(frame_size);
            for (auto & pkt : pkts) {
              auto recv = pkt.serialize_to_string();
              fECMultiReceiver.receive_pkt(recv);
            }
          }
          auto frame_logs = loggerReceiver.get_frame_logs();
          uint16_t frame_pos = 0;
          for (auto log : frame_logs) {
            if (log.not_decoded_) {
              assert(false);
            }
            assert(log.all_data_received_ == ((frame_pos <= (2*tau + 1)) or
                  !data_lost.at(frame_pos)));
            uint16_t del = 0;
            if ((tau > 0) and rs_packetization) {
              while (((frame_pos + del) % (tau + 1)) != tau) {
                del++;
              }
            }
            if ((frame_pos <= 2*tau + 1) or !data_lost.at(frame_pos)) { del = 0; }
            if (log.decoded_after_frames_ != del) {
              assert(false);
            }
            frame_pos++;
          }
        }
      }
    }
  }
  return true;
}

bool test_multi_loss_single_frame()
{
  srand(0);
  for (uint16_t size_factor = 1; size_factor < 3; size_factor++) {
    for (uint16_t trial = 0; trial < 20; trial++) {
      for (uint16_t tau = 0; tau < 4; tau++) {
        for (uint16_t rs_packetization = 0; rs_packetization < 2; rs_packetization++) {
          uint16_t seed = 0;
          uint16_t w = 32;
          uint16_t max_frame_size_possible = 12500; 
          uint16_t packet_size = 8 * size_factor;
          uint16_t stripe_size = packet_size * w;
          uint16_t max_data_frame_pkts = 64;
          uint16_t max_fec_frame_pkts = 32;
          uint16_t max_data_stripes_per_frame = 64;
          uint16_t max_fec_stripes_per_frame = 32;
          if ((tau > 1) and rs_packetization) {
            max_fec_stripes_per_frame *= tau;
            max_fec_frame_pkts *= tau;
          }
          assert (max_data_stripes_per_frame * stripe_size >= max_frame_size_possible);
          assert (max_fec_stripes_per_frame * stripe_size >= max_frame_size_possible / 2);
          assert (pow(2, w) >= uint64_t(num_frames_for_delay(tau)) * uint64_t(num_frames_for_delay(tau))
              * max_data_stripes_per_frame * max_fec_stripes_per_frame);
          uint16_t num_frames = num_frames_for_delay(tau);
          uint16_t n_cols = num_frames;
          uint16_t n_rows = 256;
          while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
          CodingMatrixInfo codingMatrixInfoHeader { n_rows, n_cols, 8};
          CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(tau) *
              max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * max_data_stripes_per_frame), w};
          Packetization * packetizationSender;
          if (rs_packetization == 0) {
            packetizationSender = new StreamingCodePacketization(w, stripe_size, map<int, pair<int, int>> { { 0 , { 0, 1} }, { 1, { 1, 2} } }, uint16_t(Packetization::MTU));
          }
          else {
            packetizationSender = new ReedSolomonMultiFramePacketization(stripe_size, map<int, pair<int, int>> { { 0 , { 0, 1} }, { 1, { 1, 2} } }, tau + 1, w, uint16_t(Packetization::MTU));
          }

          BlockCode blockCodeHeaderSender(codingMatrixInfoHeader, tau,
                                          pair<uint16_t, uint16_t>{1, 0}, 8, false);
          MultiFECHeaderCode headerCodeSender(&blockCodeHeaderSender, tau,
              codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));
          BlockCode blockCodeFECSender(codingMatrixInfoFEC, tau,
                                       pair<uint16_t, uint16_t>{1, rs_packetization == 0}, packet_size, false);
          StreamingCode codeSender(tau, stripe_size,  &blockCodeFECSender, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);

          std::vector<MetricLogger *> metricLoggersSender;
          Logger loggerSender("tests/", metricLoggersSender);
          FrameGenerator frameGenerator(&codeSender, tau, packetizationSender,
              &headerCodeSender, &loggerSender, seed);
          FECSender fECSender(frameGenerator, tau, &loggerSender, 1);
          FeedbackDatagram feedback {1};
          fECSender.handle_feedback(feedback.serialize_to_string());
          BlockCode blockCodeHeaderReceiver(codingMatrixInfoHeader, tau,
                                            pair<uint16_t, uint16_t>{1, 0}, 8, false);
          MultiFECHeaderCode headerCodeReceiver(&blockCodeHeaderReceiver, tau,
              codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));
          BlockCode blockCodeFECReceiver(codingMatrixInfoFEC, tau,
                                         pair<uint16_t, uint16_t>{1, rs_packetization == 0}, packet_size, false);
          StreamingCode codeReceiver(tau, stripe_size,  &blockCodeFECReceiver, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);

          vector<MetricLogger *> metricLoggersReceiver;
          TimingLogger timingLoggerReceiver("test_log_receiver/");
          FrameLogger frameLoggerReceiver("test_log_receiver/");
          metricLoggersReceiver.push_back(&timingLoggerReceiver);
          Logger loggerReceiver("test_log_receiver/", metricLoggersReceiver, &frameLoggerReceiver);
          InputConverter * inputConverter = nullptr;
          Model * model = nullptr;
          QualityReportGenerator * simpleQualityReportGenerator;
          QualityReporter qualityReporter(model, inputConverter, simpleQualityReportGenerator);
          FECMultiReceiver fECMultiReceiver(&codeReceiver, tau, num_frames_for_delay(tau), &headerCodeReceiver, &qualityReporter,
              &loggerReceiver, 1);
          vector<string> frames;
          vector<uint16_t> frame_sizes;
          vector<bool> data_lost;
          for (uint16_t frame_num = 0; frame_num < 7 * (tau + 1) + 1; frame_num++) {
            frames.push_back("");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            uint16_t frame_size = 3000 + (rand() % 2000);
            if ((trial % 2) == 0) {
              frame_size = (frame_size / stripe_size) * stripe_size;
            }
            frame_sizes.push_back(frame_size);
            auto pkts = fECSender.next_frame(frame_size);
            uint16_t num_parity = 0;
            for (const auto pkt : pkts) { num_parity += pkt.is_parity; }
            auto stripes_per_pkt = pkts.front().payload.size() / stripe_size;
            uint16_t num_missing = (trial % 2) == 0? rand() % (num_parity + 1) : ((frame_size / stripe_size / 2) - 1) / stripes_per_pkt;
            auto missing = get_missing(pkts.size(), num_missing);
            bool any_data_missing = false;
            uint16_t highest_pos = 0;
            for (uint16_t pos = 0; pos < pkts.size(); pos++) {
              if (!missing.at(pos)) { highest_pos = pos; }
              else { any_data_missing |= !pkts.at(pos).is_parity; }
            }
            data_lost.push_back(any_data_missing);
            for (const auto pkt : pkts) {
              auto recv = pkt.serialize_to_string();
              if (!pkt.is_parity) { frames.at(frame_num).append(pkt.payload); }
              if ((missing.at(pkt.pos_in_frame)) and (frame_num > 2*tau + 1)) { continue; }
              fECMultiReceiver.receive_pkt(recv);
              if ((pkt.pos_in_frame == highest_pos) and (frame_num > 2*tau + 1)
                  and (!rs_packetization or ((frame_num % (tau + 1)) == tau))) {
                for (uint16_t check_frame = frame_num >= tau? frame_num - tau : 0; check_frame <= frame_num; check_frame++) {
                  auto is_recovered = fECMultiReceiver.is_frame_recovered(check_frame);
                  assert(is_recovered.has_value());
                  if(!fECMultiReceiver.is_frame_recovered(check_frame).value()) {
                    assert(false);
                  }
                  const auto recovery = fECMultiReceiver.recovered_frame(check_frame);
                  assert(recovery.size() == frame_sizes.at(check_frame));
                  auto frame_val = frames.at(check_frame);
                  for (uint16_t pos = 0; pos < frame_sizes.at(check_frame); pos++) {
                    if (recovery.at(pos) != frame_val.at(pos)) {
                      assert(false);
                    }
                  }
                }
              }
            }
          }
          for (uint16_t j = 0; j < num_frames_for_delay(tau); j++)
          {
            uint16_t frame_size = 3000 + 200 * j;
            if ((trial % 2) == 0) {
              frame_size = (frame_size / stripe_size) * stripe_size;
            }
            auto pkts = fECSender.next_frame(frame_size);
            for (auto & pkt : pkts) {
              auto recv = pkt.serialize_to_string();
              fECMultiReceiver.receive_pkt(recv);
            }
          }
          auto frame_logs = loggerReceiver.get_frame_logs();
          uint16_t frame_pos = 0;
          for (auto log : frame_logs) {
            if (log.not_decoded_) {
              assert(false);
            }
            assert(log.all_data_received_ == ((frame_pos <= 2*tau + 1)
                  or !data_lost.at(frame_pos)));
            uint16_t del = 0;
            if ((tau > 0) and rs_packetization) {
              while (((frame_pos + del) % (tau + 1)) != tau) {
                del++;
              }
            }
            if ((frame_pos <= 2*tau + 1) or !data_lost.at(frame_pos)) { del = 0; }
            if (log.decoded_after_frames_ != del) {
              assert(false);
            }
            frame_pos++;
          }
        }
      }
    }
  }
  return true;
}

bool test_multi_loss_single_frame_non_recoverable()
{
  srand(0);
  for (uint16_t size_factor = 1; size_factor < 3; size_factor++) {
    for (uint16_t trial = 0; trial < 20; trial++) {
      for (uint16_t tau = 0; tau < 4; tau++) {
        for (uint16_t rs_packetization = 0; rs_packetization < 2; rs_packetization++) {
          uint16_t seed = 0;
          uint16_t w = 32;
          uint16_t max_frame_size_possible = 12500; 
          uint16_t packet_size = 8 * size_factor;
          uint16_t stripe_size = packet_size * w;
          uint16_t max_data_frame_pkts = 64;
          uint16_t max_fec_frame_pkts = 32;
          uint16_t max_data_stripes_per_frame = 64;
          uint16_t max_fec_stripes_per_frame = 32;
          if ((tau > 1) and rs_packetization) {
            max_fec_stripes_per_frame *= tau;
            max_fec_frame_pkts *= tau;
          }
          assert (max_data_stripes_per_frame * stripe_size >= max_frame_size_possible);
          assert (max_fec_stripes_per_frame * stripe_size >= max_frame_size_possible / 2);
          assert (pow(2, w) >= uint64_t(num_frames_for_delay(tau)) * uint64_t(num_frames_for_delay(tau))
              * max_data_stripes_per_frame * max_fec_stripes_per_frame);
          uint16_t num_frames = num_frames_for_delay(tau);
          uint16_t n_cols = num_frames;
          uint16_t n_rows = 256;
          while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
          CodingMatrixInfo codingMatrixInfoHeader { n_rows, n_cols, 8};
          CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(tau) *
              max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * max_data_stripes_per_frame), w};
          Packetization * packetizationSender;
          if (rs_packetization == 0) {
            packetizationSender = new StreamingCodePacketization(w, stripe_size, map<int, pair<int, int>> { { 0 , { 0, 1} }, { 1, { 1, 2} } }, uint16_t(Packetization::MTU));
          }
          else {
            packetizationSender = new ReedSolomonMultiFramePacketization(stripe_size, map<int, pair<int, int>> { { 0 , { 0, 1} }, { 1, { 1, 2} } }, tau + 1, w, uint16_t(Packetization::MTU));
          }

          BlockCode blockCodeHeaderSender(codingMatrixInfoHeader, tau,
                                          pair<uint16_t, uint16_t>{1, 0}, 8, false);
          MultiFECHeaderCode headerCodeSender(&blockCodeHeaderSender, tau,
              codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));
          BlockCode blockCodeFECSender(codingMatrixInfoFEC, tau,
                                       pair<uint16_t, uint16_t>{1, rs_packetization == 0}, packet_size, false);
          StreamingCode codeSender(tau, stripe_size,  &blockCodeFECSender, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);

          std::vector<MetricLogger *> metricLoggersSender;
          Logger loggerSender("tests/", metricLoggersSender);
          FrameGenerator frameGenerator(&codeSender, tau, packetizationSender,
              &headerCodeSender, &loggerSender, seed);
          FECSender fECSender(frameGenerator, tau, &loggerSender, 1);
          FeedbackDatagram feedback {1};
          fECSender.handle_feedback(feedback.serialize_to_string());
          BlockCode blockCodeHeaderReceiver(codingMatrixInfoHeader, tau,
                                            pair<uint16_t, uint16_t>{1, 0}, 8, false);
          MultiFECHeaderCode headerCodeReceiver(&blockCodeHeaderReceiver, tau,
              codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));
          BlockCode blockCodeFECReceiver(codingMatrixInfoFEC, tau,
                                         pair<uint16_t, uint16_t>{1, rs_packetization == 0}, packet_size, false);
          StreamingCode codeReceiver(tau, stripe_size,  &blockCodeFECReceiver, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);

          vector<MetricLogger *> metricLoggersReceiver;
          TimingLogger timingLoggerReceiver("test_log_receiver/");
          FrameLogger frameLoggerReceiver("test_log_receiver/");
          metricLoggersReceiver.push_back(&timingLoggerReceiver);
          Logger loggerReceiver("test_log_receiver/", metricLoggersReceiver, &frameLoggerReceiver);
          InputConverter * inputConverter = nullptr;
          Model * model = nullptr;
          QualityReportGenerator * simpleQualityReportGenerator;
          QualityReporter qualityReporter(model, inputConverter, simpleQualityReportGenerator);
          FECMultiReceiver fECMultiReceiver(&codeReceiver, tau, num_frames_for_delay(tau), &headerCodeReceiver, &qualityReporter,
              &loggerReceiver, 1);
          vector<string> frames;
          vector<uint16_t> frame_sizes;
          for (uint16_t frame_num = 0; frame_num < 7 * (tau + 1) + 1; frame_num++) {
            frames.push_back("");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            uint16_t frame_size = 3000 + (rand() % 2000);
            if ((trial % 2) == 0) {
              frame_size = (frame_size / stripe_size) * stripe_size;
            }
            frame_sizes.push_back(frame_size);
            auto pkts = fECSender.next_frame(frame_size);
            uint16_t num_parity = 0;
            for (const auto pkt : pkts) { num_parity += pkt.is_parity; }
            auto stripes_per_pkt = pkts.front().payload.size() / stripe_size;
            uint16_t num_missing = (trial % 2) == 0? num_parity + 1 + (rand() % (pkts.size() - num_parity)) : 1 + ((frame_size / stripe_size / 2 + 1)) / stripes_per_pkt;
            num_missing = max(int(num_parity + 1), int(num_missing));
            num_missing = min(int(pkts.size() -1), int(num_missing));
            auto missing = get_missing(pkts.size(), num_missing);
            uint16_t highest_pos = 0;
            for (uint16_t pos = 0; pos < pkts.size(); pos++) {
              if (!missing.at(pos)) { highest_pos = pos; }
            }
            for (const auto pkt : pkts) {
              auto recv = pkt.serialize_to_string();
              if (!pkt.is_parity) { frames.at(frame_num).append(pkt.payload); }
              if (missing.at(pkt.pos_in_frame)) { continue; }
              fECMultiReceiver.receive_pkt(recv);
              if (pkt.pos_in_frame == highest_pos) {
                for (uint16_t check_frame = frame_num >= tau? frame_num - tau : 0; check_frame <= frame_num; check_frame++) {
                  auto is_recovered = fECMultiReceiver.is_frame_recovered(check_frame);
                  assert(is_recovered.has_value());
                  if(fECMultiReceiver.is_frame_recovered(check_frame).value()) {
                    assert(false);
                  }
                }
              }
            }
          }
          for (uint16_t j = 0; j < num_frames_for_delay(tau); j++)
          {
            uint16_t frame_size = 3000 + 200 * j;
            if ((trial % 2) == 0) {
              frame_size = (frame_size / stripe_size) * stripe_size;
            }
            auto pkts = fECSender.next_frame(frame_size);
            for (auto & pkt : pkts) {
              if (!pkt.is_parity) {
                auto recv = pkt.serialize_to_string();
                fECMultiReceiver.receive_pkt(recv);
              }
            }
          }
          auto frame_logs = loggerReceiver.get_frame_logs();
          uint16_t frame_pos = 0;
          for (auto log : frame_logs) {
            if (frame_pos < 7 * (tau + 1) + 1) {
              assert(log.not_decoded_);
              assert(!log.all_data_received_);
            }
            frame_pos++;
          }
        }
      }
    }
  }
  return true;
}

bool test_burst_loss_frame()
{
  srand(0);
  uint16_t tau = 3;
  for (uint16_t size_factor = 1; size_factor < 3; size_factor++) {
    for (uint16_t trial = 0; trial < 5; trial++) {
      for (uint16_t burst_start = 4; burst_start < 18; burst_start++) {
        uint16_t seed = 0;
        uint16_t w = 32;
        uint16_t max_frame_size_possible = 10000; 
        uint16_t packet_size = 8 * size_factor;
        uint16_t stripe_size = packet_size * w;
        uint16_t max_data_frame_pkts = 100;
        uint16_t max_fec_frame_pkts = 75;
        uint16_t max_data_stripes_per_frame = 100;
        uint16_t max_fec_stripes_per_frame = 75;
        assert (max_data_stripes_per_frame * stripe_size >= max_frame_size_possible);
        assert (max_fec_stripes_per_frame * stripe_size >= max_frame_size_possible / 2);
        assert((pow(2, w) > uint64_t(num_frames_for_delay(tau)) * uint64_t(num_frames_for_delay(tau))
            * max_data_stripes_per_frame * max_fec_stripes_per_frame));
        uint16_t num_frames = num_frames_for_delay(tau);
        uint16_t n_cols = num_frames;
        uint16_t n_rows = 256;
        while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
        CodingMatrixInfo codingMatrixInfoHeader { n_rows, n_cols, 8};
        CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(tau) *
            max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * max_data_stripes_per_frame), w};
        for (uint16_t rs_packetization = 0; rs_packetization < 2; rs_packetization++) {
          Packetization * packetizationSender;
          if (rs_packetization == 0) {
            packetizationSender = new StreamingCodePacketization(w, stripe_size, map<int, pair<int, int>> { { 0 , { 0, 1} }, { 1, { 1, 2} } }, uint16_t(Packetization::MTU));
          }
          else {
            packetizationSender = new ReedSolomonMultiFramePacketization(stripe_size, map<int, pair<int, int>> { { 0 , { 0, 1} }, { 1, { 1, 2} } }, tau + 1, w,
                uint16_t(Packetization::MTU));
          }
          BlockCode blockCodeHeaderSender(codingMatrixInfoHeader, tau,
                                          pair<uint16_t, uint16_t>{1, 0}, 8, false);
          MultiFECHeaderCode headerCodeSender(&blockCodeHeaderSender, tau,
              codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));
          BlockCode blockCodeFECSender(codingMatrixInfoFEC, tau,
                                       pair<uint16_t, uint16_t>{1, rs_packetization == 0}, packet_size, false);
          StreamingCode codeSender(tau, stripe_size,  &blockCodeFECSender, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);

          std::vector<MetricLogger *> metricLoggersSender;
          Logger loggerSender("tests/", metricLoggersSender);
          FrameGenerator frameGenerator(&codeSender, tau, packetizationSender,
              &headerCodeSender, &loggerSender, seed);
          FECSender fECSender(frameGenerator, tau, &loggerSender, 1);
          FeedbackDatagram feedback {1};
          fECSender.handle_feedback(feedback.serialize_to_string());
          BlockCode blockCodeHeaderReceiver(codingMatrixInfoHeader, tau,
                                            pair<uint16_t, uint16_t>{1, 0}, 8, false);
          MultiFECHeaderCode headerCodeReceiver(&blockCodeHeaderReceiver, tau,
              codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));
          BlockCode blockCodeFECReceiver(codingMatrixInfoFEC, tau,
                                         pair<uint16_t, uint16_t>{1, rs_packetization == 0}, packet_size, false);
          StreamingCode codeReceiver(tau, stripe_size, &blockCodeFECReceiver, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);

          vector<MetricLogger *> metricLoggersReceiver;
          TimingLogger timingLoggerReceiver("test_log_receiver/");
          FrameLogger frameLoggerReceiver("test_log_receiver/");
          metricLoggersReceiver.push_back(&timingLoggerReceiver);
          Logger loggerReceiver("test_log_receiver/", metricLoggersReceiver, &frameLoggerReceiver);
          InputConverter * inputConverter = nullptr;
          Model * model = nullptr;
          QualityReportGenerator * simpleQualityReportGenerator;
          QualityReporter qualityReporter(model, inputConverter, simpleQualityReportGenerator);
          FECMultiReceiver fECMultiReceiver(&codeReceiver, tau, num_frames_for_delay(tau), &headerCodeReceiver, &qualityReporter,
              &loggerReceiver, 1);
          vector<string> frames;
          vector<uint16_t> frame_sizes;
          vector<bool> frames_recovered(2, true);
          if ((rs_packetization == 1) and ((burst_start % 4) == 2)) {
            frames_recovered = { false, false };
          }
          if ((rs_packetization == 1) and ((burst_start % 4) == 3)) {
            frames_recovered = { false, true };
          }
          for (uint16_t frame_num = 0; frame_num <= burst_start + tau +1; frame_num++) {
            frames.push_back("");
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            uint16_t frame_size = 2 * stripe_size + (rand() % 1000);
            if (frame_num > burst_start + 1) { frame_size = 2* frame_size + 6*  stripe_size; }
            if ((trial % 2) == 0) {
              frame_size = (frame_size / stripe_size) * stripe_size;
            }
            frame_sizes.push_back(frame_size);
            auto pkts = fECSender.next_frame(frame_size);
            for (auto & pkt : pkts) {
              auto recv = pkt.serialize_to_string();
              if (!pkt.is_parity) { frames.at(frame_num).append(pkt.payload); }
              if ((frame_num == burst_start) or (frame_num == (burst_start + 1))) { continue; }
              fECMultiReceiver.receive_pkt(recv);
              if ((pkt.pos_in_frame == (pkts.size() - 1)) and (frame_num == (burst_start + tau + 1))) {
                for (uint16_t rec_frame = burst_start; rec_frame < burst_start + 2; rec_frame++) {
                  auto is_recovered = fECMultiReceiver.is_frame_recovered(rec_frame);
                  if (!is_recovered.has_value()) {
                    assert(false);
                  }
                  if ((rs_packetization == 1) and ((burst_start % 4) == 2)) {
                    if (is_recovered.value()) {
                      assert(false);
                    }
                  }
                  else if ((rs_packetization == 1) and ((burst_start % 4) == 3) and (burst_start != rec_frame - 1)) {
                    if (is_recovered.value()) {
                      assert(false);
                    }
                  }
                  else if ((rs_packetization == 1) and (((burst_start % 4) == 1))) {
                    continue;
                  }
                  else {
                    const auto recovery = fECMultiReceiver.recovered_frame(rec_frame);
                    if (recovery.size() != frame_sizes.at(rec_frame)) {
                      assert(false);
                    }
                    auto frame_val = frames.at(rec_frame);
                    for (uint16_t pos = 0; pos < frame_sizes.at(rec_frame); pos++) {
                      if (recovery.at(pos) != frame_val.at(pos)) {
                        assert(false);
                      }
                    }
                  }
                }
              }
            }
          }
          for (uint16_t j = 0; j < num_frames_for_delay(tau); j++)
          {
            uint16_t frame_size = 3000 + 200 * j;
            if ((trial % 2) == 0) {
              frame_size = (frame_size / stripe_size) * stripe_size;
            }
            auto pkts = fECSender.next_frame(frame_size);
            for (auto & pkt : pkts) {
              auto recv = pkt.serialize_to_string();
              fECMultiReceiver.receive_pkt(recv);
            }
          }
          auto frame_logs = loggerReceiver.get_frame_logs();
          uint16_t frame_pos = 0;
          for (auto log : frame_logs) {
            if ((rs_packetization == 1) and (((burst_start % 4) == 1))) {
              frame_pos++;
              continue;
            }
            bool decoded = true;
            if (frame_pos == burst_start) {
              decoded = frames_recovered.front();
              assert(!log.all_data_received_);
            }
            else if (frame_pos == burst_start + 1) {
              decoded = frames_recovered.back();
              assert(!log.all_data_received_);
            }
            else {
              assert(log.all_data_received_);
            }
            if (log.not_decoded_ == decoded) {
              assert(false);
            }
            uint16_t del = 0;
            if (rs_packetization) {
              while (((frame_pos + del) % (tau + 1)) != tau) {
                del++;
              }
            }
            else { del = tau; }
            if ((frame_pos != burst_start) and (frame_pos != (burst_start + 1))) { del = 0; }
            if (decoded and (log.decoded_after_frames_ != del)) {
              assert(false);
            }
            frame_pos++;
          }
        }
      }
    }
  }
  return true;
}

int main(int /* argc */, char ** /* argv[] */)
{
  assert(test_simple_encode_decode_no_loss());
  printf("test_simple_encode_decode: passed\n");
  assert(test_loss_single_frame());
  printf("test_loss_single_frame: passed\n");
  assert(test_multi_loss_single_frame());
  printf("test_multi_loss_single_frame: passed\n");
  assert(test_multi_loss_single_frame_non_recoverable());
  printf("test_multi_loss_single_frame_non_recoverable: passed\n");
  assert(test_burst_loss_frame());
  printf("test_burst_loss_frame: passed\n");
  return 0;
}
