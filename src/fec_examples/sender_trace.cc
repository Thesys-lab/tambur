/* UDP client that demonstrates nonblocking I/O with Epoller */
#include <cassert>
#include <iostream>
#include <fstream>
#include <string>
#include <deque>
#include <unistd.h>
#include <stdlib.h>
#include <cstdlib>
#include <filesystem>

#include "conversion.hh"
#include "timestamp.hh"
#include "udp_socket.hh"
#include "epoller.hh"
#include "fec_sender.hh"
#include "frame_generator.hh"
#include "streaming_code.hh"
#include "ge_channel.hh"
#include "multi_fec_header_code.hh"
#include "reed_solomon_multi_frame_packetization.hh"
#include "logger.hh"
#include "metric_logger.hh"
#include "timing_logger.hh"
#include "json.hpp"
#include "parse_cfg.hh"

using namespace std;
using json = nlohmann::json;

int main(int argc, char * argv[])
{
  if (argc != 4 && argc != 9) {
    cerr << "Usage: " << argv[0] << " host config trace [ seed t_gb_ t_bg_ l_g_ l_b_ ]" << endl;
    return EXIT_FAILURE;
  }
  Config::parse(argv[2]);

  Address peer_addr{argv[1], narrow_cast<uint16_t>(Config::port)};
  std::ifstream json_trace_file(argv[3]);

  json trace;
  json_trace_file >> trace;

  std::vector<int> packet_parities, packet_sizes;
  std::vector<uint16_t> frame_numbers;

  packet_parities = trace["packet_parities"].get<std::vector<int>>();
  packet_sizes = trace["packet_sizes"].get<std::vector<int>>();
  frame_numbers = trace["frame_numbers"].get<std::vector<uint16_t>>();

  uint16_t tau = Config::tau;

  std::vector<uint64_t> frame_sizes;
  for (uint64_t i = 0; i < num_frames_for_delay(Config::tau); ++i)
  {
    frame_sizes.push_back(1);
  }

  uint16_t prev_frame = 0;
  for (size_t i = 0; i < packet_parities.size(); ++i)
  {
      if (packet_parities[i] == 0)
      {
          if (frame_numbers[i] == prev_frame) {
            frame_sizes.back() += packet_sizes[i];
          }
          else
          {
            prev_frame = frame_numbers[i];
            frame_sizes.push_back(packet_sizes[i]);
          }
      }
  }
  assert(Config::sender_port != Config::port);
  UDPSocket udp_sock;
  udp_sock.bind({"0", narrow_cast<uint16_t>(Config::sender_port)});
  udp_sock.connect(peer_addr);
  cerr << "Local address: " << udp_sock.local_address().str() << endl;

  // set socket to be nonblocking
  udp_sock.set_blocking(false);

  Epoller epoller;
  deque<FECDatagram> datagram_buf;


  auto log_folder = Config::exp_log_folder;
  log_folder += "/" + Config::cfg_file_base_name + "/" + Config::coding_scheme + "_" + fs::path(argv[3]).filename().string() + "/";
  if (log_folder.substr(log_folder.size() - 6, 6) == ".json/") {
    log_folder = log_folder.substr(0,log_folder.size()-6) + "/";
  }
  fs::create_directory(log_folder);
  vector<MetricLogger *> metricLoggersFrameGenerator;

  TimingLogger timingLoggerFrameGenerator(log_folder + "frame_generator/");
  metricLoggersFrameGenerator.push_back(&timingLoggerFrameGenerator);

  Logger loggerFrameGenerator(log_folder + "frame_generator/", metricLoggersFrameGenerator);

  vector<MetricLogger *> metricLoggersFECSender;
  TimingLogger timingLoggerFECSender(log_folder + "sender/");
  metricLoggersFECSender.push_back(&timingLoggerFECSender);
  Logger loggerFECSender(log_folder + "sender/", metricLoggersFECSender);


  CodingMatrixInfo codingMatrixInfoHeader;
  CodingMatrixInfo codingMatrixInfoFEC;
  optional<BlockCode> blockCodeHeader;
  optional<BlockCode> blockCodeFECSender;

  Code* code;
  Packetization* packetization;
  HeaderCode* headerCode;
  if (Config::coding_scheme.rfind("ReedSolomonMultiFrame", 0) == 0) {
    uint16_t num_frames = num_frames_for_delay(Config::tau);
    uint16_t n_cols = num_frames;
    uint16_t n_rows = 256;
    while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
    codingMatrixInfoHeader = CodingMatrixInfo { n_rows, n_cols, 8};

    codingMatrixInfoFEC = CodingMatrixInfo{uint16_t(num_frames_for_delay(tau) *
      Config::max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * Config::max_data_stripes_per_frame), Config::w};
    packetization = new ReedSolomonMultiFramePacketization(Config::stripe_size, map<int, pair<int, int>> { { 0 , { 0, 1} }, { Config::max_qr, { 1, 2} } }, tau + 1, Config::w,
                uint16_t(Packetization::MTU));

    blockCodeHeader.emplace(codingMatrixInfoHeader, tau,
      pair<uint16_t, uint16_t>{1, 0 }, 8, false);

    headerCode = new MultiFECHeaderCode(&blockCodeHeader.value(), tau,codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));

    blockCodeFECSender.emplace(codingMatrixInfoFEC, tau,
      pair<uint16_t, uint16_t>{1, 0 }, Config::packet_size, false, &loggerFECSender);
    code = new StreamingCode(tau, Config::stripe_size, &blockCodeFECSender.value(), Config::w, Config::max_data_stripes_per_frame, Config::max_fec_stripes_per_frame);
  } else if (Config::coding_scheme.rfind("StreamingCode", 0) == 0) {
    packetization = new StreamingCodePacketization(Config::w, Config::stripe_size, map<int, pair<int, int>> { { 0 , { 0, 1} }, { Config::max_qr, { 1, 2} }, { 2 * Config::max_qr, { 1, 4} } }, uint16_t(Packetization::MTU), Config::parity_delay);

    uint16_t num_frames = num_frames_for_delay(Config::tau);
    uint16_t n_cols = num_frames;
    uint16_t n_rows = 256;
    while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
    codingMatrixInfoHeader = CodingMatrixInfo { n_rows, n_cols, 8};

    codingMatrixInfoFEC = CodingMatrixInfo{uint16_t(num_frames_for_delay(tau) *
      Config::max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * Config::max_data_stripes_per_frame), Config::w};

    blockCodeHeader.emplace(codingMatrixInfoHeader, tau,
      pair<uint16_t, uint16_t>{1, 0 }, 8, false);

    blockCodeFECSender.emplace(codingMatrixInfoFEC, tau,
      pair<uint16_t, uint16_t>{1, 1 }, Config::packet_size, false, &loggerFECSender);

    headerCode = new MultiFECHeaderCode(&blockCodeHeader.value(), tau,codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));

    code = new StreamingCode(tau, Config::stripe_size, &blockCodeFECSender.value(), Config::w, Config::max_data_stripes_per_frame, Config::max_fec_stripes_per_frame);
  } else {
    std::cerr << "Invalid coding_scheme " << Config::coding_scheme << "inside the config\n";
    return 1;
  }
  float p_good_to_bad = 0.02;
  float p_bad_to_good = 0.5;
  float p_loss_good = 0.01;
  float p_loss_bad = .5;
  int seed = 1;


  FrameGenerator frameGenerator(code, tau, packetization,
      headerCode, &loggerFrameGenerator, 0);


  FECSender fECSender(frameGenerator, tau, &loggerFECSender, 33);

  bool all_pkts_sent = false;
  std::string const SEND_PKT_STR = "send_pkt";
  std::string const FRAME_SIZE_STR = "frame_size";
  int frame_data_size = 0, frame_par_size = 0, qr = 0;
  prev_frame = 0;

  // if UDP socket is readable
  epoller.register_event(udp_sock.fd(), Epoller::In,
    [&udp_sock, &fECSender]()
    {
      FeedbackDatagram feedbackDatagram;
      auto data = udp_sock.recv();
      if (not feedbackDatagram.parse_from_string(data)) {
        throw runtime_error("failed to parse a feedback datagram");
      }
      fECSender.handle_feedback(data);
    }
  );

  // if UDP socket is writable
  epoller.register_event(udp_sock.fd(), Epoller::Out,
    [&epoller, &udp_sock, &datagram_buf, &all_pkts_sent, &loggerFECSender, &SEND_PKT_STR, &FRAME_SIZE_STR, &frame_data_size, &frame_par_size, &prev_frame, &qr, &fECSender]()
    {
      while (not datagram_buf.empty()) {
        const auto & datagram = datagram_buf.front();
        auto data_str = datagram.serialize_to_string();

        if (udp_sock.send(data_str)) {
          loggerFECSender.log_event(SEND_PKT_STR, datagram.frame_num);
          if (datagram.frame_num != prev_frame) {
            loggerFECSender.log_value(FRAME_SIZE_STR, prev_frame, frame_data_size, frame_par_size, qr);
            prev_frame = datagram.frame_num;
            frame_data_size = datagram.is_parity ? 0 : datagram.payload.size();
            frame_par_size  = datagram.is_parity ? datagram.payload.size() : 0;
            qr = fECSender.get_state();
          } else {
            frame_data_size += datagram.is_parity ? 0 : datagram.payload.size();
            frame_par_size  += datagram.is_parity ? datagram.payload.size() : 0;
          }
          datagram_buf.pop_front();
        } else { // EWOULDBLOCK
          cerr << "Try again later" << endl;
          break;
        }
      }

      if (datagram_buf.empty()) {
        // not interested in socket being writable anymore since buf is empty
        if (all_pkts_sent) {
          FECDatagram lastPkt;
          lastPkt.payload = "[SENTINEL]";
          udp_sock.send(lastPkt.serialize_to_string());
          std::cerr << "Sent [SENTINEL] packet\n";
          loggerFECSender.log_value(FRAME_SIZE_STR, prev_frame, frame_data_size, frame_par_size);
        }
        epoller.deactivate(udp_sock.fd(), Epoller::Out);
      }
    }
  );

  uint16_t frame_pos = 0;
  uint16_t num_frames = frame_sizes.size();

  uint64_t total = 0;
  for (auto fs : frame_sizes) {
    total += fs;
  }
  total -= num_frames_for_delay(Config::tau);

  if (argc > 4) {
    seed = atoi(argv[4]);
    p_good_to_bad = atof(argv[5]);
    p_bad_to_good = atof(argv[6]);
    p_loss_good = atof(argv[7]);
    p_loss_bad = atof(argv[8]);
  }

  seed =  total % 65536;

  std::cerr << seed << " " << p_good_to_bad << " " << p_bad_to_good << " "
            << p_loss_good << " " << p_loss_bad << std::endl;

  GEChannel gE(p_good_to_bad,p_bad_to_good,p_loss_good,p_loss_bad,seed);

  while (frame_pos < num_frames) {
    if (fECSender.frame_ready()) {
      gE.transition();
      bool sent_something = false;
      auto size = frame_sizes.at(frame_pos++);
      uint64_t max_frame_size = (Config::max_frame_size_possible / Config::stripe_size) * Config::stripe_size;
      uint64_t rel_num_frames = (size / max_frame_size) + ((size % max_frame_size) > 0);
      vector<uint16_t> frames;
      for (uint64_t j = 0; j < rel_num_frames; j++) {
        uint16_t sz = uint16_t(size / rel_num_frames) + uint16_t(j < (size % rel_num_frames));
        frames.push_back(sz);
      }
      for (uint16_t rel_frame_pos = 0; rel_frame_pos < frames.size(); rel_frame_pos++) {
        auto frame_size = frames.at(rel_frame_pos);
        const auto pkts = fECSender.next_frame(frame_size, rel_frame_pos ==
            (frames.size() - 1));
        for (uint16_t i = 0; i < pkts.size(); i++) {
          if ((pkts.front().frame_num < num_frames_for_delay(Config::tau))
              or gE.is_received()) {
            sent_something = true;
            datagram_buf.emplace_back(pkts.at(i));
          }
        }
      }
      // interested in socket being writable again since buf becomes nonempty
      if (sent_something) { epoller.activate(udp_sock.fd(), Epoller::Out); }
    }

    epoller.poll(fECSender.time_remaining());
  }

  all_pkts_sent = true;
  epoller.activate(udp_sock.fd(), Epoller::Out);
  epoller.poll(1);
  epoller.poll(1);

  sleep(5);

  return EXIT_SUCCESS;
}
