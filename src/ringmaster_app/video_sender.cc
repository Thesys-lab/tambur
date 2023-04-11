// Modified from https://github.com/microsoft/ringmaster/blob/main/src/app/video_sender.cc
#include <getopt.h>
#include <iostream>
#include <string>
#include <memory>
#include <stdexcept>
#include <utility>
#include <chrono>

#include "conversion.hh"
#include "timerfd.hh"
#include "udp_socket.hh"
#include "poller.hh"
#include "yuv4mpeg.hh"
#include "protocol.hh"
#include "encoder.hh"
#include "timestamp.hh"

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
using namespace chrono;
using json = nlohmann::json;

// global variables in an unnamed namespace
namespace {
  constexpr unsigned int BILLION = 1000 * 1000 * 1000;
}

void print_usage(const string & program_name)
{
  cerr <<
  "Usage: " << program_name << " [options] port y4m\n\n"
  "Options:\n"
  "--mtu <MTU>                MTU for deciding UDP payload size\n"
  "-o, --output <file>        file to output performance results to\n"
  "-v, --verbose              enable more logging for debugging"
  << endl;
}

pair<Address, ConfigMsg> recv_config_msg(UDPSocket & udp_sock)
{
  // wait until a valid ConfigMsg is received
  while (true) {
    const auto & [peer_addr, raw_data] = udp_sock.recvfrom();

    const shared_ptr<Msg> msg = Msg::parse_from_string(raw_data.value());
    if (msg == nullptr or msg->type != Msg::Type::CONFIG) {
      continue; // ignore invalid or non-config messages
    }

    const auto config_msg = dynamic_pointer_cast<ConfigMsg>(msg);
    if (config_msg) {
      return {peer_addr, *config_msg};
    }
  }
}

int main(int argc, char * argv[])
{
  bool not_all_pkts_sent = true;
  // argument parsing
  string output_path;
  string y4m_path;
  bool verbose = false;
  const option cmd_line_opts[] = {
    {"config", required_argument, nullptr, 'C'},
    {"mtu",     required_argument, nullptr, 'M'},
    {"video", required_argument, nullptr, 'V'},
    {"output",  required_argument, nullptr, 'o'},
    {"verbose", no_argument,       nullptr, 'v'},
    { nullptr,  0,                 nullptr,  0 },
  };
  string config;
  while (true) {
    const int opt = getopt_long(argc, argv, "o:v", cmd_line_opts, nullptr);
    if (opt == -1) {
      break;
    }

    switch (opt) {
      case 'M':
        Datagram::set_mtu(strict_stoi(optarg));
        break;
      case 'C':
        config = optarg;
        break;
      case 'V':
        y4m_path = optarg;
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
  int seed = Config::seed;
  float t_gb = Config::t_gb;
  float t_bg = Config::t_bg;
  float l_g = Config::l_g;
  float l_b = Config::l_b;

  auto log_folder = Config::experiment_log_folder;
  log_folder += "/" + Config::cfg_file_base_name.substr(0, Config::cfg_file_base_name.find("_tmp")) + "/" + Config::coding_scheme + "_" + Config::video_name + "/";
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

  Code *code;
  Packetization *packetization;
  HeaderCode *headerCode;

  uint16_t tau = Config::tau;

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
    packetization = new ReedSolomonMultiFramePacketization(Config::stripe_size, map<int, pair<int, int>>{{0, {0, 1}}, {Config::max_qr, {1, 2}}}, tau + 1, Config::w,
                                                           uint16_t(Packetization::MTU));

    blockCodeHeader.emplace(codingMatrixInfoHeader, tau,
                            pair<uint16_t, uint16_t>{1, 0}, 8, false);

    headerCode = new MultiFECHeaderCode(&blockCodeHeader.value(), tau, codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));

    blockCodeFECSender.emplace(codingMatrixInfoFEC, tau,
                               pair<uint16_t, uint16_t>{1, 0}, Config::packet_size, false, &loggerFECSender);

    code = new StreamingCode(tau, Config::stripe_size, &blockCodeFECSender.value(), Config::w, Config::max_data_stripes_per_frame, Config::max_fec_stripes_per_frame);
  }
  else if (Config::coding_scheme.rfind("StreamingCode", 0) == 0)
  {
    packetization = new StreamingCodePacketization(Config::w, Config::stripe_size, map<int, pair<int, int>>{{0, {0, 1}}, {Config::max_qr, {1, 2}}, {2 * Config::max_qr, {1, 4}}}, uint16_t(Packetization::MTU), Config::parity_delay);

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

    blockCodeFECSender.emplace(codingMatrixInfoFEC, tau,
                               pair<uint16_t, uint16_t>{1, 1}, Config::packet_size, false, &loggerFECSender);

    headerCode = new MultiFECHeaderCode(&blockCodeHeader.value(), tau, codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));

    code = new StreamingCode(tau, Config::stripe_size, &blockCodeFECSender.value(), Config::w, Config::max_data_stripes_per_frame, Config::max_fec_stripes_per_frame);
  }
  else
  {
    std::cerr << "Invalid coding_scheme " << Config::coding_scheme << "inside the config\n";
    return 1;
  }

  FrameGenerator frameGenerator(code, tau, packetization,
                                headerCode, &loggerFrameGenerator, 0);
  FECSender fECSender(frameGenerator, tau, &loggerFECSender, 33,
                      Config::max_data_stripes_per_frame * Config::stripe_size, true);
  GEChannel gE(t_gb, t_bg, l_g, l_b, seed);

  const auto port = narrow_cast<uint16_t>(Config::sender_port);

  UDPSocket udp_sock;
  udp_sock.bind({"0", port});
  cerr << "Local address: " << udp_sock.local_address().str() << endl;

  // wait for a receiver to send 'ConfigMsg' and "connect" to it
  cerr << "Waiting for receiver..." << endl;

  const auto & [peer_addr, config_msg] = recv_config_msg(udp_sock);
  cerr << "Peer address: " << peer_addr.str() << endl;
  udp_sock.connect(peer_addr);

  // read configuration from the peer
  const auto width = config_msg.width;
  const auto height = config_msg.height;
  const auto frame_rate = config_msg.frame_rate;
  const auto target_bitrate = config_msg.target_bitrate;

  cerr << "Received config: width=" << to_string(width)
       << " height=" << to_string(height)
       << " FPS=" << to_string(frame_rate)
       << " bitrate=" << to_string(target_bitrate) << endl;

  uint64_t start = timestamp_ms();

  // set UDP socket to non-blocking now
  udp_sock.set_blocking(false);

  // open the video file
  YUV4MPEG video_input(y4m_path, width, height);

  // allocate a raw image
  RawImage raw_img(width, height);

  // initialize the encoder
  Encoder encoder(width, height, frame_rate, target_bitrate, output_path,
                  &fECSender, &gE, Config::tau, &loggerFECSender);
  encoder.set_verbose(verbose);

  Poller poller;

  // create a periodic timer with the same period as the frame interval
  Timerfd fps_timer;
  const timespec frame_interval {0, static_cast<long>(BILLION / frame_rate)};
  fps_timer.set_time(frame_interval, frame_interval);

  // read a raw frame when the periodic timer fires
  poller.register_event(fps_timer, Poller::In,
    [&]()
    {
      // being lenient: read raw frames 'num_exp' times and use the last one
      const auto num_exp = fps_timer.read_expirations();
      if (num_exp > 1 and verbose)
      {
        cerr << "Warning: skipping " << num_exp - 1 << " raw frames" << endl;
      }

      for (unsigned int i = 0; i < num_exp; i++)
      {
        // fetch a raw frame into 'raw_img' from the video input
        if (not video_input.read_frame(raw_img) or (((timestamp_ms() - start) / 1000) >= Config::timeout))
        {
          not_all_pkts_sent = false;
          encoder.send_buf().emplace_back(Datagram{encoder.frame_id(), encoder.frame_type(), 0, 0, "[SENTINAL TERMINATE NOW]"});
        }
      }
      if (not_all_pkts_sent)
      {
        // compress 'raw_img' into frame 'frame_id' and packetize it
        encoder.compress_frame(raw_img);
      }
      // interested in socket being writable if there are datagrams to send
      if (not encoder.send_buf().empty())
      {
        poller.activate(udp_sock, Poller::Out);
      }
    });

  // when UDP socket is writable
  poller.register_event(udp_sock, Poller::Out,
    [&]()
    {
      deque<Datagram> &send_buf = encoder.send_buf();
      while (not send_buf.empty())
      {
        auto &datagram = send_buf.front();

        // timestamp the sending time before sending
        datagram.send_ts = timestamp_us();
        if (udp_sock.send(datagram.serialize_to_string()))
        {
          if (not not_all_pkts_sent)
          {
            cerr << "Terminated " << endl;
            send_buf.pop_front();
            break;
          }
          if (verbose and false)
          {
            cerr << "Sent datagram: frame_id=" << datagram.frame_id
                  << " frag_id=" << datagram.frag_id
                  << " frag_cnt=" << datagram.frag_cnt
                  << " rtx=" << datagram.num_rtx << endl;
          }

          // move the sent datagram to unacked if not a retransmission
          if (datagram.num_rtx == 0)
          {
            FECDatagram fec_pkt;
            fec_pkt.parse_header_from_string(datagram.payload);
            encoder.add_unacked_frame(datagram.frame_id, fec_pkt.frame_num);
            encoder.add_unacked(move(datagram));
          }

          send_buf.pop_front();
        }
        else
        {                       // EWOULDBLOCK; try again later
          datagram.send_ts = 0; // since it wasn't sent successfully
          break;
        }
      }
      // not interested in socket being writable if no datagrams to send
      if (send_buf.empty())
      {
        poller.deactivate(udp_sock, Poller::Out);
      }
    });

  // when UDP socket is readable
  poller.register_event(udp_sock, Poller::In,
    [&]()
    {
      while (true) {
        const auto & raw_data = udp_sock.recv();

        if (not raw_data) { // EWOULDBLOCK; try again when data is available
          break;
        }
        const shared_ptr<Msg> msg = Msg::parse_from_string(*raw_data);

        // ignore invalid or non-ACK messages
        if (msg == nullptr or ((msg->type != Msg::Type::ACK) and (msg->type != Msg::Type::FEC) and (msg->type != Msg::Type::FRAME))) {
          return;
        }
        else if (msg->type == Msg::Type::FRAME)
        {
          const auto frame = dynamic_pointer_cast<FRAMEMsg>(msg);
          if (verbose)
          {
            cerr << "Received Frame feedback: frame_id=" << frame->frame_id << endl;
          }
          encoder.handle_frame_feedback(frame);
        }
        else if (msg->type == Msg::Type::FEC)
        {
          const auto fec = dynamic_pointer_cast<FECMsg>(msg);
          if (verbose)
          {
            cerr << "Received FEC feedback: info=" << (uint16_t)fec->info
                  << endl;
          }
          encoder.handle_fec_feedback(fec);
        }
        else
        {
          const auto ack = dynamic_pointer_cast<AckMsg>(msg);
          // RTT estimation, retransmission, etc.
          encoder.handle_ack(ack);
          if (verbose)
          {
            cerr << "Received ACK: frame_id=" << ack->frame_id
                  << " frag_id=" << ack->frag_id << endl;
          }
        }
      }
      if (not encoder.send_buf().empty())
      {
        poller.activate(udp_sock, Poller::Out);
      }
    });

  // create a periodic timer for outputting stats every second
  Timerfd stats_timer;
  const timespec stats_interval {1, 0};
  stats_timer.set_time(stats_interval, stats_interval);

  poller.register_event(stats_timer, Poller::In,
    [&]()
    {
      if (stats_timer.read_expirations() == 0) {
        return;
      }

      // output stats every second
      encoder.output_periodic_stats();
    }
  );

  // main loop
  while (not_all_pkts_sent)
  {
    poller.poll(-1);
  }
  sleep(1);
  poller.poll(-1);
  sleep(1);
  cerr << "Sender terminated" << endl;
  cerr << "Sender log folder " << log_folder << endl;
  return EXIT_SUCCESS;
}
