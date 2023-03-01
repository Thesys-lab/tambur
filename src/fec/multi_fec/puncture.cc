
#include "puncture.hh"

using namespace std;

uint16_t get_puncture_dim(uint16_t delay, uint16_t num_rows,
    const vector<bool>  & retained_frames)
{
  auto num_frames = num_frames_for_delay(delay);
  uint16_t num_frames_used = 0;
  for (auto frame_used : retained_frames) { num_frames_used += frame_used; }
  return (num_rows / num_frames) * num_frames_used;
}

vector<pair<uint16_t, uint16_t>> get_kept_rows(
    CodingMatrixInfo codingMatrixInfo, uint16_t delay, uint16_t num_frames,
    const vector<bool> & retained_frames, uint16_t n_rows)
{
  vector<pair<uint16_t, uint16_t>> kept_rows;
  if (n_rows == 0) { return kept_rows; }
  kept_rows.reserve(n_rows);
  for (uint16_t j = 0; j < num_frames; j++) {
    if (retained_frames.at(j)) {
      auto row_field_elements = rows_of_frame(j, delay, codingMatrixInfo);
      auto first = row_field_elements.first;
      auto second = first + (row_field_elements.second -
          row_field_elements.first + 1) - 1;
      kept_rows.push_back({ first, second });
    }
  }
  return kept_rows;
}
