#include "streaming_code_helper.hh"

using namespace std;

StreamingCodeHelper::StreamingCodeHelper(CodingMatrixInfo codingMatrixInfo,
    uint16_t delay, pair<uint16_t, uint16_t> v_to_u_ratio) :
    codingMatrixInfo_(codingMatrixInfo), delay_(delay)
{
  set_vs_and_us(v_to_u_ratio);
}

void StreamingCodeHelper::set_vs_and_us(pair<uint16_t, uint16_t> v_to_u_ratio)
{
  positions_of_us_ = vector<bool>(codingMatrixInfo_.n_cols, false);
  auto num_frames = num_frames_for_delay(delay_);
  for (uint16_t frame_num = 0; frame_num < num_frames; frame_num++) {
    auto cols = cols_of_frame(frame_num, delay_, codingMatrixInfo_);
    for (uint16_t col = cols.first; col <= cols.second; col++) {
      if ((col - cols.first) % (v_to_u_ratio.first + v_to_u_ratio.second)
          >= v_to_u_ratio.first) { positions_of_us_.at(col) = true; }
    }
  }
}

bool StreamingCodeHelper::zero_by_frame_u(Position position,
    CodingMatrixInfo codingMatrixInfo)
{
  if (zero_by_frame(position, codingMatrixInfo, delay_)) { return true; }
  if (!positions_of_us_.at(position.col)) { return false; }
  uint16_t parity_frame = frame_of_row(position.row, delay_,
      codingMatrixInfo);
  uint16_t data_frame = frame_of_col(position.col, delay_,
      codingMatrixInfo);
  auto num_frames = num_frames_for_delay(delay_);
  return (parity_frame != data_frame) and
      (parity_frame  != ((delay_ + data_frame) % num_frames));
}

vector<bool> StreamingCodeHelper::get_retained_parities_zero_by_memory(
    uint16_t timeslot)
{
  vector<bool> retained_parities(codingMatrixInfo_.n_rows, true);
  for (uint16_t j = 1; j <= delay_; j++) {
    auto endpoints = rows_of_frame(timeslot + j, delay_, codingMatrixInfo_);
    for (uint16_t i = endpoints.first; i <= endpoints.second; i++) {
      retained_parities.at(i) = false;
    }
  }
  return retained_parities;
}

vector<uint16_t> StreamingCodeHelper::lost_data_count(bool is_u,
    vector<bool> erased)
{
  auto num_frames = num_frames_for_delay(delay_);
  vector<uint16_t> lost_per_frame;
  for (uint16_t f = 0; f < num_frames; f++) { lost_per_frame.push_back(0); }
  for (uint16_t j = 0; j < num_frames; j++) {
    auto endpoints = cols_of_frame(j, delay_,
    codingMatrixInfo_);
    for (uint16_t i = endpoints.first; i <= endpoints.second; i++) {
      if (erased.at(i) and (positions_of_us_.at(i) == is_u)) {
        lost_per_frame.at(j) += 1;
      }
    }
  }
  return lost_per_frame;
}

vector<uint16_t> StreamingCodeHelper::received_parities_count(
      vector<bool> erased)
{
  auto num_frames = num_frames_for_delay(delay_);
  vector<uint16_t> received_parities_per_frame(num_frames,0);
  for (uint16_t j = 0; j < num_frames; j++) {
    auto endpoints = rows_of_frame(j, delay_, codingMatrixInfo_);
    for (uint16_t i = endpoints.first; i <= endpoints.second; i++) {
      if (!erased.at(codingMatrixInfo_.n_cols + i)) {
        received_parities_per_frame.at(j) += 1;
      }
    }
  }
  return received_parities_per_frame;
}

pair<vector<bool>, vector<bool>> StreamingCodeHelper::get_recoverable_data(
    vector<bool> erased, uint16_t timeslot, bool check_recover)
{
  auto num_frames = num_frames_for_delay(delay_);
  vector<uint16_t> lost_us_per_frame = lost_data_count(true, erased);
  vector<uint16_t> lost_vs_per_frame = lost_data_count(false, erased);
  vector<uint16_t> received_parities_per_frame = received_parities_count(
        erased);
  streamingCode_.emplace(delay_);
  streamingCode_->add_frames(lost_us_per_frame, lost_vs_per_frame,
      received_parities_per_frame, timeslot);
  return get_retained_info(num_frames, erased, timeslot,
      streamingCode_.value(), check_recover);
}

vector<bool> StreamingCodeHelper::get_recoverable_data_info(
    uint16_t num_frames, vector<bool> erased, vector<bool> recovered_us,
    vector<bool> recovered_vs)
{
  vector<bool> recoverable_data(codingMatrixInfo_.n_cols, false);
  for (uint16_t j = 0; j < num_frames; j++) {
    if (recovered_us.at(j) or recovered_vs.at(j)) {
      auto endpoints = cols_of_frame(j, delay_, codingMatrixInfo_);
      for (uint16_t i = endpoints.first; i <= endpoints.second; i++) {
        recoverable_data.at(i) = (positions_of_us_.at(i) ? recovered_us.at(j) :
            recovered_vs.at(j)) and erased.at(i);
      }
    }
  }
  return recoverable_data;
}

vector<bool> StreamingCodeHelper::get_retained_parity_info(vector<bool> erased,
    uint16_t timeslot, vector<bool> unusable_parities,
    vector<uint16_t> used_parities_counts)
{
  auto n_cols = codingMatrixInfo_.n_cols;
  auto retained_parities = get_retained_parities_zero_by_memory(timeslot);
  for (uint16_t j = 0; j < retained_parities.size(); j++) {
    uint16_t frame = frame_of_row(j, delay_, codingMatrixInfo_);
    if (erased.at(j+n_cols) or unusable_parities.at(frame) or
        used_parities_counts.at(frame) == 0) {
      retained_parities.at(j) = false;
    }
    else  {
      used_parities_counts.at(frame)--;
    }
  }
  return retained_parities;
}

pair<vector<bool>, vector<bool>> StreamingCodeHelper::get_retained_info(
      uint16_t num_frames, vector<bool> erased, uint16_t timeslot,
      StreamingCodeTheoretical streamingCode, bool check_recover)
{
  auto u_recovered_unordered = streamingCode.frame_u_recovered();
  auto v_recovered_unordered = streamingCode.frame_v_recovered();
  auto unusable_parities_unordered = streamingCode.frame_unusable_parities();
  auto u = lost_data_count(true, erased);
  auto v = lost_data_count(false, erased);
  auto p = received_parities_count(erased);
  vector<uint16_t> lost_data_u, lost_data_v, received_parity;
  for (uint16_t j = timeslot + 1; j < timeslot + 1 + num_frames; j++) {
    lost_data_u.push_back(u.at(j % num_frames));
    lost_data_v.push_back(v.at(j % num_frames));
    received_parity.push_back(p.at(j % num_frames));
  }
  auto used_parities_counts_unordered = check_recover ? received_parity :
      get_used_parity_counts(lost_data_u, lost_data_v, received_parity,
      u_recovered_unordered, v_recovered_unordered,
      unusable_parities_unordered);
  auto used_parities_counts = reorder_to_matrix_positions<uint16_t>(timeslot,
      used_parities_counts_unordered, num_frames);
  auto unusable_parities = reorder_to_matrix_positions<bool>(timeslot,
      unusable_parities_unordered, num_frames);
  auto recovered_us = reorder_to_matrix_positions<bool>(timeslot,
      u_recovered_unordered, num_frames);
  auto recovered_vs = reorder_to_matrix_positions<bool>(timeslot,
      v_recovered_unordered, num_frames);
  auto retained_parities = get_retained_parity_info(erased, timeslot,
      unusable_parities, used_parities_counts);
  auto recoverable_data = get_recoverable_data_info(
      num_frames, erased, recovered_us, recovered_vs);
  return { recoverable_data, retained_parities };
}
