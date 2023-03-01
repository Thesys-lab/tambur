#ifndef FEC_MULTI_RECEIVER_HH
#define FEC_MULTI_RECEIVER_HH

#include <map>
#include <optional>
#include <deque>

#include "code.hh"
#include "frame.hh"
#include "header_code.hh"
#include "feedback_datagram.hh"
#include "quality_reporter.hh"
#include "logger.hh"
#include "logging/frame_log.hh"

class FECMultiReceiver
{
public:
  FECMultiReceiver(Code * code, uint8_t tau, uint8_t memory,
      HeaderCode * headerCode, QualityReporter * qualityReporter,
      Logger * logger, uint64_t interval_ms = 2000, bool pad = false,
      bool video_frame_info_padded = false);

  bool feedback_ready() const;

  void terminate();

  // see how much time is left before the next feedback should be generated
  uint64_t time_remaining() const;

  FeedbackDatagram get_feedback();

  std::optional<std::vector<FECDatagram>> get_frame_pkts(uint16_t frame_num)
      const;

  bool receive_pkt(const std::string & binary_datagram);

  bool frame_expired(uint16_t new_frame_num, uint16_t old_frame_num) const;

  std::optional<bool> is_frame_recovered(uint16_t frame_num);

  std::string recovered_frame(uint16_t frame_num);

  bool video_frame_info_available(uint16_t frame_num);

  std::tuple<uint16_t, uint8_t, uint8_t> video_frame_info(uint16_t frame_num);

  std::vector<std::pair<uint16_t, uint32_t>> recovered_video_frames();

  uint16_t get_num_frames() { return memory_; }

private:
  int index_of_frame(uint16_t frame_num) const;

  void add_new_frame(const FECDatagram & pkt, bool is_valid_pkt);

  void purge_frames(uint16_t frame_num);

  void place_pkt_in_frame(const FECDatagram & pkt);

  void update_frame_sizes(
      const std::vector<std::pair<uint16_t, uint64_t>> & frame_sizes_and_valid);

  void update_payloads(
      const std::vector<uint16_t> & payloads_and_sqns);

  void decode_headers(FECDatagram const& pkt);

  void decode_payloads(FECDatagram const& pkt);

  void fill_missing_frames(const FECDatagram& pkt);

private:
  Code * code_;
  uint8_t tau_;
  uint8_t memory_; // Expected to be 2*tau_+1
  HeaderCode * headerCode_;
  QualityReporter * qualityReporter_;
  Logger * logger_;
  uint64_t target_;
  uint64_t interval_ms_;
  std::deque<Frame> frames_;
  std::deque<FrameLog> frames_log_;
  bool received_first_frame_{false};
  uint16_t last_received_frame_{0};
  bool video_frame_info_padded_;
};

#endif /* FEC_MULTI_RECEIVER_HH */
