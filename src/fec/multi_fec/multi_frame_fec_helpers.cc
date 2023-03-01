#include <stdexcept>
#include <cassert>
#include <cmath>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <iterator>

#include "multi_frame_fec_helpers.hh"
#include "packetization.hh"
#include "serialization.hh"

using namespace std;

void cauchy_bit_matrix(Matrix<int> & matrix, CodingMatrixInfo codingMatrixInfo,
    uint16_t delay, vector<vector<bool>> zero_streaming)
{
  int* tmp_matrix = cauchy_original_coding_matrix(codingMatrixInfo.n_cols,
      codingMatrixInfo.n_rows, codingMatrixInfo.w);
  for (uint16_t i = 0; i < codingMatrixInfo.n_rows ; i++) {
    for (uint16_t j = 0; j < codingMatrixInfo.n_cols; j++) {
      if (zero_by_frame(Position { i , j}, codingMatrixInfo, delay)
          or (i < zero_streaming.size() and j < zero_streaming.at(i).size()
          and zero_streaming.at(i).at(j))
          ) {
        tmp_matrix[i*codingMatrixInfo.n_cols + j] = 0;
      }
    }
  }
  matrix.resize(codingMatrixInfo.n_rows * codingMatrixInfo.w,
      codingMatrixInfo.n_cols * codingMatrixInfo.w);
  int* tmp_bit_matrix = jerasure_matrix_to_bitmatrix(codingMatrixInfo.n_cols,
      codingMatrixInfo.n_rows, codingMatrixInfo.w, tmp_matrix);
  uint16_t index = 0;
  for (uint16_t i = 0; i < codingMatrixInfo.n_rows * codingMatrixInfo.w; i++) {
    for (uint16_t j = 0; j < codingMatrixInfo.n_cols * codingMatrixInfo.w; j++) {
      matrix.at(i, j) = tmp_bit_matrix[index++];
    }
  }
  free(tmp_matrix);
  free(tmp_bit_matrix);
}

void set_cauchy_matrix(Matrix<int> & matrix, CodingMatrixInfo codingMatrixInfo,
    uint16_t delay, vector<vector<bool>> zero_streaming)
{
  int* tmp_matrix = cauchy_original_coding_matrix(codingMatrixInfo.n_cols,
      codingMatrixInfo.n_rows, codingMatrixInfo.w);
  for (uint16_t i = 0; i < codingMatrixInfo.n_rows ; i++) {
    for (uint16_t j = 0; j < codingMatrixInfo.n_cols; j++) {
      if (zero_by_frame(Position { i , j}, codingMatrixInfo, delay)
          or (i < zero_streaming.size() and j < zero_streaming.at(i).size()
          and zero_streaming.at(i).at(j))
          ) {
        tmp_matrix[i*codingMatrixInfo.n_cols + j] = 0;
      }
      else {
        auto par_frame = frame_of_row(i, delay, codingMatrixInfo);
        auto encoded_frame = frame_of_col(j, delay, codingMatrixInfo);
        for (uint16_t l = par_frame + 1; l <= par_frame + delay; l++) {
          assert((l % num_frames_for_delay(delay)) != (encoded_frame % num_frames_for_delay(delay)));
        }
      }
    }
  }
  matrix.resize(codingMatrixInfo.n_rows,
      codingMatrixInfo.n_cols);
  uint64_t index = 0;
  for (uint16_t i = 0; i < codingMatrixInfo.n_rows; i++) {
    for (uint16_t j = 0; j < codingMatrixInfo.n_cols; j++) {
      matrix.at(i, j) = tmp_matrix[index++];
    }
  }
  free(tmp_matrix);
}

bool zero_by_frame(Position position, CodingMatrixInfo codingMatrixInfo,
    uint16_t delay)
{
  uint16_t num_frames = num_frames_for_delay(delay);
  uint16_t encode_for_frame = frame_of_row(position.row, delay,
      codingMatrixInfo);
  uint16_t to_include = frame_of_col(position.col, delay, codingMatrixInfo);
  for (uint16_t j = 0; j < delay + 1; j++) {
    if (((to_include + j) % num_frames) == encode_for_frame) { return false; }
  }
  return true;
}

pair<uint16_t, uint16_t> rows_of_frame(uint16_t frame_num, uint16_t delay,
    CodingMatrixInfo codingMatrixInfo)
{
  uint16_t frame_position = frame_num % num_frames_for_delay(delay);
  auto n_rows_per_frame = num_rows_per_frame(codingMatrixInfo, delay);
  uint16_t start = frame_position * n_rows_per_frame;
  uint16_t end = start + n_rows_per_frame - 1;
  return pair<uint16_t, uint16_t>{ start, end };
}

std::pair<uint16_t, uint16_t> cols_of_frame(uint16_t frame_num, uint16_t delay,
    CodingMatrixInfo codingMatrixInfo)
{
  uint16_t frame_position = frame_num % num_frames_for_delay(delay);
  auto n_cols_per_frame = num_cols_per_frame(codingMatrixInfo, delay);
  uint16_t start = frame_position * n_cols_per_frame;
  uint16_t end = start + n_cols_per_frame - 1;
  return pair<uint16_t, uint16_t>{ start, end };
}

uint16_t frame_of_row(uint16_t row_num, uint16_t delay,
    CodingMatrixInfo codingMatrixInfo)
{
  auto num_frames = num_frames_for_delay(delay);
  if (row_num < codingMatrixInfo.n_rows) {
    if (num_frames == 1) { return 0; }
    for (uint16_t frame_num = 0; frame_num < num_frames;
        frame_num++) {
      auto rows = rows_of_frame(frame_num, delay, codingMatrixInfo);
      if (row_num >= rows.first and row_num <= rows.second) {
        return frame_num;
      }
    }
  }
  throw runtime_error("Invalid row number\n");
  return 0;
}

uint16_t frame_of_col(uint16_t col_num, uint16_t delay,
    CodingMatrixInfo codingMatrixInfo)
{
  for (uint16_t frame_num = 0; frame_num < num_frames_for_delay(delay);
      frame_num++) {
    auto cols = cols_of_frame(frame_num, delay, codingMatrixInfo);
    if (col_num >= cols.first and col_num <= cols.second) {
      return frame_num;
    }
  }
  throw runtime_error("Invalid column number\n");
  return 0;
}

vector<bool> get_retained_frames(uint16_t val, uint16_t n_frames)
{
  vector<bool> retained_frames(n_frames, false);
  for (uint16_t pos = 0; pos < n_frames; pos++) {
    retained_frames.at(pos) = (val / uint16_t(pow(2,pos))) % 2;
  }
  return retained_frames;
}

vector<string> convert_sizes_to_FEC_input(const vector<uint64_t> & sizes)
{
  vector<string> data;
  data.reserve(sizes.size());
  for (uint16_t index = 0; index < sizes.size(); index++) {
    data.push_back(put_number(sizes.at(index)));
  }
  convert_strings_to_FEC_inputs(data);
  return data;
}

void convert_strings_to_FEC_inputs(vector<string> & input_vals)
{
  for (uint16_t i = 0; i < input_vals.size(); i++) {
    convert_string_to_FEC_input(input_vals.at(i));
  }
}

void convert_string_to_FEC_input(string & input_val)
{
  string padding;
  if ((input_val.size() % sizeof(long)) > 0) {
    padding = string(sizeof(long) - (input_val.size() % sizeof(long)), '0');
    input_val.reserve(input_val.size() + padding.size());
    input_val.insert(input_val.end(),padding.begin(),padding.end());
  }
}
