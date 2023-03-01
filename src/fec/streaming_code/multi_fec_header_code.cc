#include <cassert>
#include <map>

#include "multi_fec_header_code.hh"
#include "wrap_helpers.hh"
#include "serialization.hh"

using namespace std;

MultiFECHeaderCode::MultiFECHeaderCode(BlockCode * blockCode, uint16_t delay,
    uint16_t max_pkts_per_frame) : timeslot_(-1), delay_(delay), num_frames_(
    num_frames_for_delay(delay)), max_pkts_per_frame_(max_pkts_per_frame),
    blockCode_(blockCode)
{
  codingMatrixInfo_ = blockCode->get_coding_matrix_info();
  if (codingMatrixInfo_.n_rows != uint16_t(num_frames_ * max_pkts_per_frame)
      or codingMatrixInfo_.n_cols != num_frames_ or codingMatrixInfo_.w != 8
      or blockCode_->get_delay() != delay_ or blockCode_->get_packet_size() !=
      8) {
    throw runtime_error("Invalid blockCode input\n");
  }
}

uint64_t MultiFECHeaderCode::encode_sizes_of_frames(uint64_t frame_size,
    uint16_t pkt_pos_in_frame )
{
  if (pkt_pos_in_frame == 0) {
    timeslot_++;
    blockCode_->update_timeslot(uint16_t(timeslot_));
    vector<char> val;
    string sz = put_number<uint64_t>(frame_size);
    for (auto el : sz) { val.push_back(char(el)); }
    blockCode_->place_payload(uint16_t(timeslot_) % num_frames_, false, val);
    blockCode_->encode();
    return frame_size;
  }
  uint16_t time = uint16_t(timeslot_) % num_frames_;
  auto encoding = blockCode_->get_row(time * max_pkts_per_frame_ +
      ((pkt_pos_in_frame - 1) % max_pkts_per_frame_), true);
  return get_number<uint64_t>(encoding) % (2 * 65536);
}

vector<pair<uint16_t, uint64_t>> MultiFECHeaderCode::decode_sizes_of_frames(
    uint16_t frame_num, uint16_t pos_in_frame, uint64_t size)
{
  if (frame_num > timeslot_) {
    uint16_t time = timeslot_ + 1;
    while (time != frame_num) {
      blockCode_->update_timeslot(time);
      time++;
    }
    blockCode_->update_timeslot(time);
    timeslot_ = int(frame_num);
  }
  auto val = put_number<uint64_t>(size);
  vector<char> payload(val.size(), char(0));
  for (uint16_t pos = 0; pos < val.size(); pos++) {
    payload.at(pos) = char(val.at(pos));
  }
  if (pos_in_frame == 0) {
    blockCode_->place_payload(frame_num % num_frames_, false, payload);
  }
  else { blockCode_->place_payload((frame_num % num_frames_) *
      max_pkts_per_frame_ + ((pos_in_frame - 1) % max_pkts_per_frame_), true, payload); }
  if (blockCode_->can_recover()) {
    return recover();
  }
  if (pos_in_frame == 0) { return {{ frame_num, size % (2 * 65536) }}; }
  return vector<pair<uint16_t, uint64_t>>();
}

vector<pair<uint16_t, uint64_t>> MultiFECHeaderCode::recover()
{
  vector<pair<uint16_t, uint64_t>> recovered_sizes;
  auto recovered = blockCode_->decode();
  for (uint16_t pos = 0; pos < recovered.size(); pos++) {
    if (recovered.at(pos)) {
      uint64_t size = get_number<uint64_t>(blockCode_->get_row(pos, false));
      uint16_t rel_frame_num = frame_of_col(pos, delay_, codingMatrixInfo_);
      uint16_t frame_num = 0;
      for (uint16_t t = uint16_t(timeslot_); t < uint16_t(timeslot_) +num_frames_;
          t++) {
        if (rel_frame_num == (t % num_frames_)) {
          frame_num = t == timeslot_? timeslot_ : t - num_frames_;
        }
      }
      recovered_sizes.push_back({ frame_num, size % (2 * 65536) });
    }
  }
  return recovered_sizes;
}
