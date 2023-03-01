#ifndef STREAMING_CODE_HELPER_HH
#define STREAMING_CODE_HELPER_HH

#include "flow_decode.hh"
#include "coding_matrix_info.hh"
#include "multi_frame_fec_helpers.hh"
#include "streaming_code_theoretical.hh"
#include "matrix.hh"

class StreamingCodeHelper
{
public:
  StreamingCodeHelper(CodingMatrixInfo codingMatrixInfo, uint16_t delay,
      std::pair<uint16_t, uint16_t> v_to_u_ratio = {1, 1});

  std::pair<std::vector<bool>, std::vector<bool>>get_recoverable_data(
      std::vector<bool> erased, uint16_t timeslot, bool check_recover = false);

  bool zero_by_frame_u(Position position, CodingMatrixInfo codingMatrixInfo);

  void set_vs_and_us(std::pair<uint16_t, uint16_t> v_to_u_ratio);
  std::vector<uint16_t> lost_data_count(bool is_u, std::vector<bool> erased);

  std::vector<uint16_t> received_parities_count(std::vector<bool> erased);

  std::vector<bool> get_retained_parities_zero_by_memory(uint16_t timeslot);

  std::pair<std::vector<bool>, std::vector<bool>> get_retained_info(
      uint16_t num_frames, std::vector<bool> erased, uint16_t timeslot,
      StreamingCodeTheoretical streamingCode, bool check_recover = false);

  std::vector<bool> get_recoverable_data_info(uint16_t num_frames,
      std::vector<bool> erased, std::vector<bool> recovered_us,
      std::vector<bool> recovered_vs);

  std::vector<bool> get_retained_parity_info(
      std::vector<bool> erased, uint16_t timeslot, std::vector<bool>
      unusable_parities, std::vector<uint16_t> used_parities_counts);

    std::vector<bool> get_retained_parity_info(
      std::vector<bool> erased, uint16_t timeslot,
      std::vector<bool> unusable_parities);

  std::vector<bool> get_positions_of_us() { return positions_of_us_; }
private:
  CodingMatrixInfo codingMatrixInfo_;
  uint16_t delay_;
  std::vector<bool> positions_of_us_;
  std::optional<StreamingCodeTheoretical> streamingCode_;
};

#endif /* STREAMING_CODE_HELPER_HH */
