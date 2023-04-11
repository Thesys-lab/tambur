// Modified from https://github.com/microsoft/ringmaster/blob/main/src/app/encoder.hh
#ifndef ENCODER_HH
#define ENCODER_HH

extern "C" {
#include <vpx/vpx_encoder.h>
#include <vpx/vp8cx.h>
}

#include <deque>
#include <map>
#include <memory>
#include <optional>

#include "exception.hh"
#include "image.hh"
#include "protocol.hh"
#include "file_descriptor.hh"
#include "fec_sender.hh"
#include "ge_channel.hh"
#include "logger.hh"

class Encoder
{
public:
  // initialize a VP9 encoder
  Encoder(const uint16_t display_width,
          const uint16_t display_height,
          const uint16_t frame_rate, const unsigned int bitrate_kbps,
          const std::string & output_path = "",
          FECSender * fECSender = nullptr, GEChannel * gE = nullptr,
          uint16_t tau = 3, Logger * loggerFECSender = nullptr);
  ~Encoder();

  // encode raw_img and packetize into datagrams
  void compress_frame(const RawImage & raw_img);

  // add a transmitted but unacked datagram (except retransmissions) to unacked
  void add_unacked(const Datagram & datagram);
  void add_unacked(Datagram && datagram);

  void add_unacked_frame(uint32_t frame_id, uint16_t fec_frame_id);

  // handle ACK
  void handle_ack(const std::shared_ptr<AckMsg> & ack);

  // handle FEC feedback
  void handle_fec_feedback(const std::shared_ptr<FECMsg> & feedback);

  // handle frame feedback
  void handle_frame_feedback(const std::shared_ptr<FRAMEMsg> & frame);

  // output stats every second and reset some of them
  void output_periodic_stats();

  // set target bitrate
  void set_target_bitrate();

  // accessors
  uint32_t frame_id() const { return frame_id_; }
  FrameType frame_type() const  { return frame_type_; }
  std::deque<Datagram> & send_buf() { return send_buf_; }
  std::map<SeqNum, Datagram> & unacked() { return unacked_; }
  Logger * loggerFECSender_;

  // mutators
  void set_verbose(const bool verbose) { verbose_ = verbose; }

  // forbid copying and moving
  Encoder(const Encoder & other) = delete;
  const Encoder & operator=(const Encoder & other) = delete;
  Encoder(Encoder && other) = delete;
  Encoder & operator=(Encoder && other) = delete;

private:
  uint16_t display_width_;
  uint16_t display_height_;
  uint16_t frame_rate_;
  FECSender * fECSender_;
  std::deque<std::pair<uint16_t, FECDatagram>> pkts_;
  std::optional<FileDescriptor> output_fd_;
  GEChannel * gE_;
  uint16_t tau_;

  // print debugging info
  bool verbose_ {false};

  // current target bitrate
  unsigned int target_bitrate_ {0};

  // VPX encoding configuration and context
  vpx_codec_enc_cfg_t cfg_ {};
  vpx_codec_ctx_t context_ {};

  // frame ID to encode
  uint32_t frame_id_ {0};

  // queue of datagrams (packetized video frames) to send
  std::deque<Datagram> send_buf_ {};

  // unacked datagrams
  std::map<SeqNum, Datagram> unacked_ {};

  // unacked frames
  std::map<uint32_t, std::pair<uint64_t, uint16_t>> unacked_frames_ {};

  std::optional<uint16_t> highest_acked_fec_frame_;

  // for logging
  FrameType frame_type_ {0};
  bool force_keyframe_ {false};
  uint16_t first_fec_frame_ {0};
  uint16_t last_fec_frame_ {0};
  uint64_t redundancy_size_ {0};

  // RTT-related
  std::optional<unsigned int> min_rtt_us_ {};
  std::optional<double> ewma_rtt_us_ {};
  static constexpr double ALPHA = 0.2;

  // performance stats
  unsigned int num_encoded_frames_ {0};
  double total_encode_time_ms_ {0.0};
  double max_encode_time_ms_ {0.0};

  // constants
  static constexpr unsigned int MAX_NUM_RTX = 0;
  static constexpr uint64_t MAX_UNACKED_US = 1000 * 1000; // 1 second

  // track RTT
  void add_rtt_sample(const unsigned int rtt_us);

  // encode the raw frame stored in 'raw_img'
  void encode_frame(const RawImage & raw_img);

  // packetize the just encoded frame (stored in context_) and return its size
  size_t packetize_encoded_frame();

  void update_highest_acked_fec_frame(uint32_t acked_video_frame_id);

  // VPX API wrappers
  template <typename ... Args>
  inline void codec_control(Args && ... args)
  {
    check_call(vpx_codec_control_(std::forward<Args>(args)...),
               VPX_CODEC_OK, "vpx_codec_control_");
  }
};

#endif /* ENCODER_HH */
