#ifndef MULTI_FRAME_FEC_HELPERS_HH
#define MULTI_FRAME_FEC_HELPERS_HH

#include<optional>

extern "C" {
#include "cauchy.h"
#include <gf_rand.h>
#include "galois.h"
#include "jerasure.h"
}

#include "matrix.hh"
#include "coding_matrix_info.hh"
#include "position.hh"

bool zero_by_frame(Position position, CodingMatrixInfo codingMatrixInfo,
    uint16_t delay);

void cauchy_bit_matrix(Matrix<int> & matrix, CodingMatrixInfo codingMatrixInfo,
    uint16_t delay, std::vector<std::vector<bool>> zero_streaming =
        std::vector<std::vector<bool>>());

void set_cauchy_matrix(Matrix<int> & matrix, CodingMatrixInfo codingMatrixInfo,
    uint16_t delay, std::vector<std::vector<bool>> zero_streaming =
        std::vector<std::vector<bool>>());

inline uint16_t num_frames_for_delay(uint16_t delay) { return 2 * delay + 1; }
inline uint16_t num_cols_per_frame(CodingMatrixInfo codingMatrixInfo,
    uint16_t delay)
    { return codingMatrixInfo.n_cols / num_frames_for_delay(delay); }
inline uint16_t num_rows_per_frame(CodingMatrixInfo codingMatrixInfo,
    uint16_t delay)
    { return codingMatrixInfo.n_rows / num_frames_for_delay(delay); }

std::pair<uint16_t, uint16_t> rows_of_frame(uint16_t frame_num, uint16_t delay,
    CodingMatrixInfo codingMatrixInfo);

std::pair<uint16_t, uint16_t> cols_of_frame(uint16_t frame_num, uint16_t delay,
    CodingMatrixInfo codingMatrixInfo);

uint16_t frame_of_row(uint16_t row_num, uint16_t delay,
    CodingMatrixInfo codingMatrixInfo);

uint16_t frame_of_col(uint16_t col_num, uint16_t delay,
    CodingMatrixInfo codingMatrixInfo);

template<typename T>
std::vector<T> reorder_to_matrix_positions(uint16_t timeslot, std::vector<T>
    vals, uint16_t num_frames)
{
  std::vector<T> reordered(num_frames, false);
  for (uint16_t j = 1; j <= num_frames; j++) {
    reordered.at((timeslot + j) % num_frames) = vals.at(j-1);
  }
  return reordered;
}

std::vector<std::string> convert_sizes_to_FEC_input(
    const std::vector<uint64_t> & sizes);

std::vector<bool> get_retained_frames(uint16_t val, uint16_t n_frames);

void convert_strings_to_FEC_inputs(
    std::vector<std::string> & input_vals);

void convert_string_to_FEC_input(std::string & input_val);

#endif /* MULTI_FRAME_FEC_HELPERS_HH */
