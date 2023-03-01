#ifndef STREAMING_CODE_THEORETICAL_HH
#define STREAMING_CODE_THEORETICAL_HH
#include <vector>
#include <tuple>
#include <stdint.h>

class StreamingCodeTheoretical
{
public:
  StreamingCodeTheoretical(uint16_t delay) : delay_(delay), num_frames_(0) {};

  void add_frame(uint16_t num_missing_u, uint16_t num_missing_v,
      uint16_t num_received_parity);

  void add_frames(std::vector<uint16_t> num_missing_us,
      std::vector<uint16_t> num_missing_vs,
      std::vector<uint16_t> num_received_parities, uint16_t timeslot);

  std::vector<bool> frame_u_recovered()
  {
    return part_of_frame_recovered(false);
  }

  std::vector<bool> frame_v_recovered()
  {
    return part_of_frame_recovered(true);
  }

  std::vector<bool> frame_unusable_parities() { return unusable_parities_; }

private:
  uint16_t delay_;
  uint16_t num_frames_;
  std::vector<uint16_t> num_missing_us_;
  std::vector<uint16_t> num_missing_vs_;
  std::vector<uint16_t> num_received_parities_;
  std::vector<bool> unusable_parities_;
  std::vector<bool> part_of_frame_recovered(bool is_v);
  std::vector<bool> decode_from(uint16_t start_pos,
      std::vector<bool> parities_out);
  bool decode_final();
  std::vector<bool> get_parities_for_round(std::vector<bool> curr_unusable);
  void decode_frame(uint16_t position);
  uint16_t net_v_parities(uint16_t position, std::vector<bool> curr_unusable);
  bool frame_decoded(uint16_t position);
  void decode_streaming_code();
  bool decode_only_u(uint16_t start, std::vector<bool> parities_out);
  std::pair<bool, bool> decoding_availabilities(
      uint16_t start, std::vector<bool> parities_out);
  std::pair<bool, bool> decode_success(uint16_t start,
      std::vector<bool> parities_out, int deficit);
  int decode_v_deficit(uint16_t start, std::vector<bool> parities_out);
  void set_unusable();
  std::vector<bool> set_unusable_from_pos(uint16_t pos,
      std::vector<bool> curr_unusable);
};

#endif /* STREAMING_CODE_THEORETICAL_HH */