// Modified from https://github.com/microsoft/ringmaster/blob/main/src/app/video_receiver.cc
#include <getopt.h>
#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <stdexcept>
#include <chrono>

#include "conversion.hh"
#include "udp_socket.hh"
#include "sdl.hh"
#include "protocol.hh"
#include "decoder.hh"

#include "torch/script.h"
#include "simple_quality_report_generator.hh"
#include "timestamp.hh"
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
using namespace chrono;
using json = nlohmann::json;

void print_usage(const string & program_name)
{
  cerr <<
  "Usage: " << program_name << " [options] host port width height\n\n"
  "Options:\n"
  "--fps <FPS>          frame rate to request from sender (default: 30)\n"
  "--cbr <bitrate>      request CBR from sender\n"
  "--lazy <level>       0: decode and display frames (default)\n"
  "                     1: decode but not display frames\n"
  "                     2: neither decode nor display frames\n"
  "-o, --output <file>  file to output performance results to\n"
  "-v, --verbose        enable more logging for debugging"
  << endl;
}

int main(int argc, char * argv[])
{
  // argument parsing
  uint16_t frame_rate = 30;
  unsigned int target_bitrate = 0; // kbps
  int lazy_level = 0;
  string output_path;
  bool verbose = false;

  const option cmd_line_opts[] = {
    {"fps",     required_argument, nullptr, 'F'},
    {"cbr",     required_argument, nullptr, 'B'},
    {"config", required_argument, nullptr, 'C'},
    {"host", required_argument, nullptr, 'H'},
    {"sender_host", required_argument, nullptr, 'S'},
    {"lazy",    required_argument, nullptr, 'L'},
    {"output",  required_argument, nullptr, 'o'},
    {"verbose", no_argument,       nullptr, 'v'},
    { nullptr,  0,                 nullptr,  0 },
  };
  
  string host;
  string sender_host;
  string config;

  while (true) {
    const int opt = getopt_long(argc, argv, "o:v", cmd_line_opts, nullptr);
    if (opt == -1) {
      break;
    }
    switch (opt) {
      case 'F':
        frame_rate = narrow_cast<uint16_t>(strict_stoi(optarg));
        break;
      case 'B':
        target_bitrate = strict_stoi(optarg);
        break;
      case 'H':
        host = optarg;
        break;
      case 'S':
        sender_host = optarg;
        break;
      case 'C':
        config = optarg;
        break;
      case 'L':
        lazy_level = strict_stoi(optarg);
        break;
      case 'o':
        output_path = optarg;
        break;
      case 'v':
        verbose = true;
        break;
      default:
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }
  }
  if (optind != argc) {
    print_usage(argv[0]);
    return EXIT_FAILURE;
  }
  Config::parse(config.c_str());

  auto coding_scheme = Config::coding_scheme;
  auto log_folder = Config::experiment_log_folder;
  uint16_t tau = Config::tau;

  log_folder += "/" + Config::cfg_file_base_name.substr(0, Config::cfg_file_base_name.find("_tmp")) + "/" + Config::coding_scheme + "_" + Config::video_name + "/";

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

  FECMultiReceiver fECMultiReceiver(code, tau, num_frames_for_delay(tau), headerCode,
                                    &qualityReporter, &logger, Config::interval_ms_feedback, true, true);
  Address peer_addr{host, Config::sender_port};
  cerr << "Peer address: " << peer_addr.str() << endl;

  // create a UDP socket and "connect" it to the peer (sender)
  UDPSocket udp_sock;
  udp_sock.connect(peer_addr);
  cerr << "Local address: " << udp_sock.local_address().str() << endl;

  // request a specific configuration
  const ConfigMsg config_msg(Config::width, Config::height, frame_rate, target_bitrate);
  udp_sock.send(config_msg.serialize_to_string());

  // initialize decoder
  Decoder decoder(Config::width, Config::height, lazy_level, output_path, &fECMultiReceiver);
  decoder.set_verbose(verbose);

  // main loop
  while (true) {
    // parse a datagram received from sender
    Datagram datagram;
    if (not datagram.parse_from_string(udp_sock.recv().value())) {
      throw runtime_error("failed to parse a datagram");
    }

    if (datagram.payload == "[SENTINAL TERMINATE NOW]") { break; }

    // send an ACK back to sender
    AckMsg ack(datagram);
    udp_sock.send(ack.serialize_to_string());

    if (verbose) {
      cerr << "Acked datagram: frame_id=" << datagram.frame_id
           << " frag_id=" << datagram.frag_id << endl;
    }

    if (decoder.fec_feedback_ready())
    {
      auto feedback = decoder.get_fec_feedback();
      udp_sock.send(feedback.serialize_to_string());
      if (verbose)
      {
        cerr << "FEC Feedback=" << (uint16_t)feedback.info << endl;
      }
    }

    // process the received datagram in the decoder
    decoder.add_datagram(move(datagram));
    vector<FRAMEMsg> decoded_frames = decoder.decoded_unacked_frames();
    for (auto decoded_frame : decoded_frames)
    {
      udp_sock.send(decoded_frame.serialize_to_string());    
      decoder.acked(decoded_frame.frame_id);
      if (verbose)
      {
        cerr << "Acked frame=" << decoded_frame.frame_id << endl;
      }
    }
    // check if the expected frame(s) is complete
    while (decoder.next_frame_complete()) {
      // depending on the lazy level, might decode and display the next frame
      decoder.consume_next_frame();
    }
  }
  decoder.terminate();
  sleep(1);
  frameLogger.~FrameLogger();
  timingLogger.~TimingLogger();
  logger.~Logger();
  cerr << "Finish decode " << endl;
  while (true) { sleep(1); }
  return EXIT_SUCCESS;
}
