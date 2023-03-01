/* UDP client that demonstrates nonblocking I/O with Epoller */

#include <cmath>
#include <iostream>
#include <string>
#include <deque>
#include <optional>
#include <chrono>
#include <filesystem>

#include "torch/script.h"
#include "simple_quality_report_generator.hh"
#include "conversion.hh"
#include "timestamp.hh"
#include "udp_socket.hh"
#include "epoller.hh"
#include "fec_multi_receiver.hh"
#include "streaming_code.hh"
#include "multi_fec_header_code.hh"
#include "logger.hh"
#include "metric_logger.hh"
#include "timing_logger.hh"
#include "frame_logger.hh"
#include "quality_reporter.hh"
#include "model.hh"
#include "json.hpp"
#include "parse_cfg.hh"

using namespace std;
namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    cerr << "Usage: " << argv[0] << " config trace" << endl;
    return EXIT_FAILURE;
  }
  Config::parse(argv[1]);
  UDPSocket udp_sock;
  udp_sock.bind({"0", narrow_cast<uint16_t>(Config::port)});
  cerr << "Local address: " << udp_sock.local_address().str() << endl;

  // set socket to be nonblocking
  // udp_sock.set_blocking(false);

  Epoller epoller;
  optional<Address> peer_addr;
  deque<FeedbackDatagram> datagram_buf;

  auto coding_scheme = Config::coding_scheme;
  auto log_folder = Config::exp_log_folder;
  uint16_t tau = Config::tau;

  log_folder += "/" + Config::cfg_file_base_name + "/" + coding_scheme + "_" + fs::path(argv[2]).filename().string() + "/";
  if (log_folder.substr(log_folder.size() - 6, 6) == ".json/") {
    log_folder = log_folder.substr(0,log_folder.size()-6) + "/";
  }
  vector<MetricLogger *> metricLoggers;
  TimingLogger timingLogger(log_folder + "receiver/");
  FrameLogger frameLogger(log_folder + "receiver/");
  metricLoggers.push_back(&timingLogger);
  Logger logger(log_folder + "receiver/", metricLoggers, &frameLogger);

  Code *code;
  HeaderCode *headerCode;

  optional<Model> model;
  optional<InputConverter> inputConverter;

  CodingMatrixInfo codingMatrixInfoHeader;
  CodingMatrixInfo codingMatrixInfoFEC;
  optional<BlockCode> blockCodeHeader;
  optional<BlockCode> blockCodeFEC;

  if (Config::coding_scheme.rfind("ReedSolomonMultiFrame", 0) == 0)
  {
    uint16_t num_frames = num_frames_for_delay(Config::tau);
    uint16_t n_cols = num_frames;
    uint16_t n_rows = 256;
    while (n_rows * n_cols > 255 or (n_rows % num_frames))
    {
      n_rows--;
    }
    codingMatrixInfoHeader = CodingMatrixInfo{n_rows, n_cols, 8};

    codingMatrixInfoFEC = CodingMatrixInfo{uint16_t(num_frames_for_delay(tau) *
                                                    Config::max_fec_stripes_per_frame),
                                           uint16_t(num_frames_for_delay(tau) * Config::max_data_stripes_per_frame), Config::w};

    blockCodeHeader.emplace(codingMatrixInfoHeader, tau,
                            pair<uint16_t, uint16_t>{1, 0}, 8, false);

    headerCode = new MultiFECHeaderCode(&blockCodeHeader.value(), tau, codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));

    blockCodeFEC.emplace(codingMatrixInfoFEC, tau,
                         pair<uint16_t, uint16_t>{1, 0}, Config::packet_size, false, &logger);

    code = new StreamingCode(tau, Config::stripe_size, &blockCodeFEC.value(), Config::w, Config::max_data_stripes_per_frame, Config::max_fec_stripes_per_frame);
  }
  else if (Config::coding_scheme.rfind("StreamingCode", 0) == 0)
  {
    uint16_t num_frames = num_frames_for_delay(Config::tau);
    uint16_t n_cols = num_frames;
    uint16_t n_rows = 256;
    while (n_rows * n_cols > 255 or (n_rows % num_frames))
    {
      n_rows--;
    }
    codingMatrixInfoHeader = CodingMatrixInfo{n_rows, n_cols, 8};

    codingMatrixInfoFEC = CodingMatrixInfo{uint16_t(num_frames_for_delay(tau) *
                                                    Config::max_fec_stripes_per_frame),
                                           uint16_t(num_frames_for_delay(tau) * Config::max_data_stripes_per_frame), Config::w};

    blockCodeHeader.emplace(codingMatrixInfoHeader, tau,
                            pair<uint16_t, uint16_t>{1, 0}, 8, false);

    blockCodeFEC.emplace(codingMatrixInfoFEC, tau,
                         pair<uint16_t, uint16_t>{1, 1}, Config::packet_size, false, &logger);

    headerCode = new MultiFECHeaderCode(&blockCodeHeader.value(), tau, codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));

    code = new StreamingCode(tau, Config::stripe_size, &blockCodeFEC.value(), Config::w, Config::max_data_stripes_per_frame, Config::max_fec_stripes_per_frame);

    if (Config::SCParams::model.find("high-BW") == string::npos and Config::SCParams::model.size() > 0)
    {
      std::cerr << "SETTING MODEL" << std::endl;
      model = Model(Config::SCParams::model, Config::max_qr + 1);
      inputConverter = InputConverter(Config::SCParams::model_mean, Config::SCParams::model_var);
    }
  }
  else
  {
    std::cerr << "Invalid coding_scheme " << Config::coding_scheme << "inside the config\n";
    return 1;
  }
  QualityReportGenerator *qualityReportGenerator;
  qualityReportGenerator = new SimpleQualityReportGenerator(Config::num_qrs_no_reduce, Config::max_qr);
  QualityReporter qualityReporter(model.has_value() ? &model.value() : nullptr, inputConverter.has_value() ? &inputConverter.value() : nullptr, qualityReportGenerator);

  FECMultiReceiver fecMultiReceiver(code, tau, num_frames_for_delay(tau), headerCode,
                                    &qualityReporter, &logger);
  bool all_pkts_received = false;
  auto lastPktTs = std::chrono::high_resolution_clock::now();

  // if UDP socket is readable
  epoller.register_event(udp_sock.fd(), Epoller::In,
                         [&udp_sock, &fecMultiReceiver, &peer_addr, &all_pkts_received, &lastPktTs]()
                         {
                           const auto [src_addr, data] = udp_sock.recvfrom();
                           lastPktTs = std::chrono::high_resolution_clock::now();
                           if (not peer_addr)
                           {
                             peer_addr = src_addr;
                           }
                           FECDatagram pkt;
                           if (not pkt.parse_from_string(data))
                           {
                             throw runtime_error("failed to parse a feedback datagram");
                           }
                           all_pkts_received = !fecMultiReceiver.receive_pkt(data);
                         });

  // if UDP socket is writable
  epoller.register_event(udp_sock.fd(), Epoller::Out,
                         [&epoller, &udp_sock, &datagram_buf, &peer_addr]()
                         {
                           while (not datagram_buf.empty() and peer_addr)
                           {
                             const auto &datagram = datagram_buf.front();

                             if (udp_sock.sendto(*peer_addr, datagram.serialize_to_string()))
                             {
                               datagram_buf.pop_front();
                             }
                             else
                             { // EWOULDBLOCK
                               cerr << "Try again later" << endl;
                               break;
                             }
                           }

                           if ((not peer_addr) or datagram_buf.empty())
                           {
                             // not interested in socket being writable anymore since buf is empty
                             epoller.deactivate(udp_sock.fd(), Epoller::Out);
                           }
                         });
  while (!all_pkts_received)
  {
    if (fecMultiReceiver.feedback_ready() and peer_addr)
    {
      datagram_buf.emplace_back(fecMultiReceiver.get_feedback());

      // interested in socket being writable again since buf becomes nonempty
      epoller.activate(udp_sock.fd(), Epoller::Out);
    }
    epoller.poll(fecMultiReceiver.time_remaining());
    auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - lastPktTs).count();
    if (elapsed > 8000000)
    {
      break;
    }
  }
  
  return EXIT_SUCCESS;
}
