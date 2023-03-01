#include <cassert>
#include <algorithm>

#include "streaming_code_theoretical.hh"
#include "multi_frame_fec_helpers.hh"

using namespace std;

void StreamingCodeTheoretical::add_frames(vector<uint16_t> num_missing_us,
      vector<uint16_t> num_missing_vs, vector<uint16_t> num_received_parities,
      uint16_t timeslot)
{
  auto num_frames = num_frames_for_delay(delay_);
  num_missing_us_.reserve(num_frames);
  num_missing_vs_.reserve(num_frames);
  num_received_parities_.reserve(num_frames);
  for (uint16_t j = timeslot + 1; j < timeslot + 1 + num_frames; j++) {
    add_frame(num_missing_us.at(j % num_frames),
        num_missing_vs.at(j % num_frames),
        num_received_parities.at(j % num_frames));
  }
}

void StreamingCodeTheoretical::add_frame(uint16_t num_missing_u,
    uint16_t num_missing_v, uint16_t num_received_parity)
{
  num_missing_us_.push_back(num_missing_u);
  num_missing_vs_.push_back(num_missing_v);
  num_received_parities_.push_back(num_received_parity);
  num_frames_ += 1;
  assert(num_frames_ <= num_frames_for_delay(delay_));
  unusable_parities_.push_back((num_frames_ <= delay_) or
      (num_received_parity == 0));
  decode_streaming_code();
  if (num_frames_ == num_frames_for_delay(delay_)) { set_unusable(); }
}

vector<bool> StreamingCodeTheoretical::part_of_frame_recovered(bool is_v)
{
  vector<bool> recovery(num_frames_, false);
  for (uint16_t j = 0; j < num_frames_; j++) {
    recovery.at(j) = (is_v? num_missing_vs_ : num_missing_us_).at(j) == 0;
  }
  return recovery;
}

void StreamingCodeTheoretical::decode_frame(uint16_t position)
{
  if ((num_missing_us_.at(position) + num_missing_vs_.at(position)) <=
      num_received_parities_.at(position)) {
    num_missing_us_.at(position) = 0;
    num_missing_vs_.at(position) = 0;
  }
}

bool StreamingCodeTheoretical::frame_decoded(uint16_t position)
{
  return num_missing_us_.at(position) == 0 and
      num_missing_vs_.at(position) == 0;
}

bool StreamingCodeTheoretical::decode_final()
{
  bool should_decode_final = true;
  for (uint16_t j = 0; j < num_frames_ - 1; j++) {
    should_decode_final = should_decode_final and frame_decoded(j);
  }
  return should_decode_final;
}

vector<bool> StreamingCodeTheoretical::get_parities_for_round(
      vector<bool> curr_unusable)
{
  vector<bool> parities_out_for_round;
  parities_out_for_round.reserve(curr_unusable.size());
  for (auto el : curr_unusable) { parities_out_for_round.push_back(el); }
  return parities_out_for_round;
}

vector<bool> StreamingCodeTheoretical::decode_from(uint16_t start_pos,
    vector<bool> parities_out)
{
  pair<bool, bool> decode_u_v = decoding_availabilities(start_pos,
      parities_out);
  if (decode_u_v.first) { num_missing_us_.at(start_pos) = 0; }
  if (decode_u_v.second) { num_missing_vs_.at(start_pos) = 0; }
  return set_unusable_from_pos(start_pos, parities_out);
}

void StreamingCodeTheoretical::decode_streaming_code()
{
  if (num_received_parities_.back() == 0) { return; }
  if (decode_final()) { decode_frame(num_frames_ - 1); }
  else {
    auto parities_out = get_parities_for_round(unusable_parities_);
    for (uint16_t start_pos = 0; start_pos < num_frames_; start_pos++) {
      parities_out = decode_from(start_pos, parities_out);
    }
  }
}

void StreamingCodeTheoretical::set_unusable()
{
  for (uint16_t pos = 0; pos < num_frames_; pos++) {
    unusable_parities_ = set_unusable_from_pos(pos, unusable_parities_);
  }
}

vector<bool> StreamingCodeTheoretical::set_unusable_from_pos(uint16_t pos,
    vector<bool> curr_unusable)
{
  vector<bool> unusable = get_parities_for_round(curr_unusable);
  if (!frame_decoded(pos)) {
    unusable.at(pos) = true;
  }
  uint16_t delay_pos = pos + delay_;
  if (num_missing_us_.at(pos) > 0) {
    if (delay_pos < num_frames_) { unusable.at(delay_pos) = true; }
  }
  if (num_missing_vs_.at(pos) > 0) {
    for (uint16_t z = pos + 1; z < min(int(num_frames_), int(delay_pos + 1));
        z++) { unusable.at(z) = true; }
  }
  return unusable;
}

uint16_t StreamingCodeTheoretical::net_v_parities(uint16_t position,
    vector<bool> curr_unusable)
{
  if (curr_unusable.at(position)) { return 0; }
  if (num_missing_us_.at(position) >= num_received_parities_.at(position)) {
    return 0;
  }
  return num_received_parities_.at(position) - num_missing_us_.at(position);
}

int StreamingCodeTheoretical::decode_v_deficit(uint16_t start,
    vector<bool> parities_out)
{
  int num_lost_v = 0;
  int available_v_parity = 0;
  for (uint16_t pos = start; pos < min(int(num_frames_),
      int(start + delay_)); pos++) {
    num_lost_v += num_missing_vs_.at(pos);
    available_v_parity += net_v_parities(pos, parities_out);
    if ((num_lost_v <= available_v_parity)) { return 0; }
  }
  return num_lost_v - available_v_parity;
}

bool StreamingCodeTheoretical::decode_only_u(uint16_t start,
    vector<bool> parities_out)
{
  int lost = int(num_missing_us_.at(start)) - int(parities_out.at(start)?
      0 : num_received_parities_.at(start));
  if (lost <= 0) { return true; }
  if ((start + delay_) >= num_frames_) { return false; }
  lost += num_missing_us_.at(start + delay_) +
      num_missing_vs_.at(start + delay_) - int(parities_out.at(start + delay_)
      ? 0 : num_received_parities_.at(start + delay_));
  return lost <= 0;
}

pair<bool, bool> StreamingCodeTheoretical::decode_success(uint16_t start,
    vector<bool> parities_out, int deficit)
{
  if ((start + delay_) >= num_frames_) { return { false, false }; }
  deficit += int(num_missing_us_.at(start) + num_missing_us_.at(start + delay_)  +
      num_missing_vs_.at(start + delay_)) - int(parities_out.at(start + delay_)
      ? 0 : num_received_parities_.at(start + delay_));
  return { deficit <= 0, deficit <= 0 };
}

pair<bool, bool> StreamingCodeTheoretical::
    decoding_availabilities(
    uint16_t start, vector<bool> parities_out)
{
  int deficit = decode_v_deficit(start, parities_out);
  if (deficit == 0) { return { decode_only_u(start, parities_out), true }; }
  else { return decode_success(start, parities_out, deficit); }
}
