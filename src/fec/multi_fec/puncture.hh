#ifndef PUNCTURE_HH
#define PUNCTURE_HH

#include <cmath>
#include <map>
#include <vector>
#include <algorithm>
#include <stdint.h>

#include "coding_matrix_info.hh"
#include "matrix.hh"
#include "multi_frame_fec_helpers.hh"

uint16_t get_puncture_dim(uint16_t delay, uint16_t num_rows,
    const std::vector<bool>  & retained_frames);

std::vector<std::pair<uint16_t, uint16_t>> get_kept_rows(
    CodingMatrixInfo codingMatrixInfo, uint16_t delay, uint16_t num_frames,
    const std::vector<bool> & retained_frames, uint16_t n_rows);

template<typename T>
void set_decode_matrix(const std::vector<std::pair<uint16_t, uint16_t>> &
    kept_rows, std::optional<Matrix<T>> & decode_matrix, Matrix<T> & new_matrix)
{
  uint16_t curr_row = 0;
  for (auto end_points : kept_rows) {
    for (uint16_t row = end_points.first; row <= end_points.second; row++) {
      for (uint16_t col = 0; col < decode_matrix->max_cols(); col++) {
        new_matrix.at(curr_row, col) =
            decode_matrix->at(row, col);
      }
      curr_row++;
    }
  }
}

template<typename T>
void get_puncture(CodingMatrixInfo codingMatrixInfo, uint16_t delay,
    std::optional<Matrix<T>> & decode_matrix, Matrix<T> & new_matrix,
    const std::vector<bool>  & retained_frames)
{
  auto num_frames = num_frames_for_delay(delay);
  const auto kept_rows = get_kept_rows(codingMatrixInfo, delay, num_frames,
      retained_frames, new_matrix.max_rows());
  set_decode_matrix<T>(kept_rows, decode_matrix, new_matrix);
}

#endif /* PUNCTURE_HH */
