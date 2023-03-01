#ifndef BLOCK_CODE
#define BLOCK_CODE

#include <optional>

#include "logger.hh"
#include "puncture.hh"
#include "block_code_factory.hh"

class BlockCode
{
public:
  BlockCode(CodingMatrixInfo codingMatrixInfo,
      uint16_t delay, std::pair<uint16_t, uint16_t> v_to_u_ratio = {1, 1},
      uint16_t packet_size = 8, bool use_bit_matrix = false, Logger* logger = nullptr);

  void coded_payload(uint16_t first_pos, uint16_t final_pos,
      const Matrix<char> & puncture_mat);

  void place_payload(uint16_t row, bool is_parity, std::vector<char> vals);

  void update_timeslot(uint16_t timeslot);

  std::string get_row(uint16_t row, bool is_parity);

  void encode();

  std::vector<bool> decode();

  bool can_recover();

  std::vector<bool> get_erased() { return erased_; }

  void pad(uint16_t frame_num, uint16_t num_data_used);

  CodingMatrixInfo get_coding_matrix_info() { return codingMatrixInfo_; }

  uint16_t get_delay() { return delay_; }

  uint16_t get_packet_size() { return packet_size_; }

private:
  std::optional<Matrix<int>> matrix_;
  std::optional<Matrix<char>> data_matrix_;
  std::optional<Matrix<char>> parity_matrix_;
  std::optional<StreamingCodeHelper> streamingCodeHelper_;
  CodingMatrixInfo codingMatrixInfo_;
  uint16_t delay_;
  uint16_t packet_size_;
  void init_matrices(const Matrix<int> & matrix, uint16_t
    num_stripes, CodingMatrixInfo codingMatrixInfo);
  void initialize_data();
  std::vector<bool> erased_;
  uint16_t timeslot_;
  bool use_bit_matrix_;
  Logger* logger_;

  void validate_matrix(std::pair<uint16_t, uint16_t> v_to_u_ratio);
};

#endif /* BLOCK_CODE */
