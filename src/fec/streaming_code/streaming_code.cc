#include <vector>
#include <optional>
#include <cmath>
#include <string>
#include <iostream>
#include <map>
#include "streaming_code.hh"
#include "multi_frame_fec_helpers.hh"
#include "wrap_helpers.hh"

using namespace std;

StreamingCode::StreamingCode(uint16_t delay, uint16_t stripe_size,
    BlockCode* blockCode, uint16_t w, uint16_t frame_max_data_stripes, uint16_t
    frame_max_fec_stripes) : delay_(delay), num_frames_(num_frames_for_delay(
    delay)), stripe_size_(stripe_size), frame_max_data_stripes_(
    frame_max_data_stripes), frame_max_fec_stripes_(frame_max_fec_stripes), blockCode_(blockCode)
{
  codingMatrixInfo_ = blockCode->get_coding_matrix_info();
  if ((codingMatrixInfo_.n_rows != uint16_t(num_frames_ * frame_max_fec_stripes))
      or (codingMatrixInfo_.n_cols != uint16_t(num_frames_ * frame_max_data_stripes))
      or (stripe_size_) != (w * blockCode_->get_packet_size())
      or blockCode_->get_delay() != delay_) {
    throw runtime_error("Invalid blockCode input\n");
  }
}

// creates payloads for parity packets
vector<string> StreamingCode::encode(const vector<FECDatagram> & data_pkts,
    const vector<uint16_t> & parity_pkt_sizes, uint16_t frame_size)
{
  if (!timeslot_.has_value()) { timeslot_ = 0; }
  blockCode_->update_timeslot(timeslot_.value());
  const string frame = get_frame(data_pkts, frame_size);
  place_frame(frame, stripe_size_, timeslot_.value(), frame_max_data_stripes_,
      frame_max_fec_stripes_, num_frames_, *blockCode_, frame_size);
  blockCode_->pad(timeslot_.value(),
      compute_num_data_stripes(frame.size(), stripe_size_));
  blockCode_->encode();
  return parity_payloads(codingMatrixInfo_, timeslot_.value()++,
      parity_pkt_sizes, stripe_size_, frame_max_fec_stripes_, *blockCode_, delay_);
}

uint16_t StreamingCode::updated_ts(uint16_t frame_num)
{
  assert(!timeslot_.has_value() or frame_num >= timeslot_);
  if (timeslot_.has_value() and (frame_num <= timeslot_.value())) {
    return timeslot_.value();
  }
  uint16_t ts = timeslot_.has_value()? timeslot_.value() + 1 : 0;
  while (ts != frame_num + 1) {
    blockCode_->update_timeslot(ts);
    ts++;
  }
  return frame_num;
}

//called whenever a packet is received only non-expired frames
void StreamingCode::add_packet(const FECDatagram & pkt, uint16_t frame_num)
{
  timeslot_ = updated_ts(frame_num);
  assert(pkt.payload.size() % stripe_size_ == 0);
  uint16_t start = first_pos_of_frame_in_block_code(pkt.frame_num, num_frames_,
      frame_max_data_stripes_, frame_max_fec_stripes_, pkt.is_parity);
  auto start_pos = start + uint16_t(pkt.stripe_pos_in_frame);
  auto num_stripes_per_pkt = pkt.payload.size() / stripe_size_;
  for (uint64_t rel_pos = 0; rel_pos < num_stripes_per_pkt; rel_pos++) {
    vector<char> stripe = get_stripe(pkt.payload, rel_pos, stripe_size_,
        stripe_size_);
    blockCode_->place_payload(start_pos + rel_pos, pkt.is_parity, stripe);
  }
}

bool StreamingCode::is_stripe_recovered(uint16_t frame_num, uint16_t stripe_num, uint16_t frame_size)
{
  assert(stripe_num < compute_num_data_stripes(frame_size, stripe_size_));
  auto first_pos = first_pos_of_frame_in_block_code_erased(frame_num, num_frames_,
      frame_max_data_stripes_, frame_max_fec_stripes_, false);
  auto pos = first_pos + stripe_num;
  return !blockCode_->get_erased().at(pos);
}

string StreamingCode::recovered_stripe(uint16_t frame_size,
    uint16_t frame_num, uint16_t stripe_num)
{
  auto start = first_pos_of_frame_in_block_code_erased(frame_num, num_frames_,
      frame_max_data_stripes_, frame_max_fec_stripes_, false);
  auto absolute_stripe = start + stripe_num;
  const auto row = blockCode_->get_row(absolute_stripe, false);
  auto sz = final_pos_within_stripe(frame_size, stripe_size_, stripe_num) + 1;
  const auto stripe = get_stripe(row, 0, sz, stripe_size_);
  return string(stripe.begin(), stripe.end());
}

bool StreamingCode::is_frame_recovered(uint16_t frame_num, uint16_t frame_size)
{
  if (expired_frame(frame_num, timeslot_.value(), num_frames_)) {
    return false;
  }
  auto num_data = compute_num_data_stripes(frame_size, stripe_size_);
  for (uint16_t stripe = 0; stripe < num_data; stripe++) {
    if (!is_stripe_recovered(frame_num, stripe, frame_size)) { return false; }
  }
  return true;
}

string StreamingCode::recovered_frame(uint16_t frame_num, uint16_t frame_size)
{
  if (!is_frame_recovered(frame_num, frame_size)) {
    throw runtime_error("Frame not recovered\n");
  }
  string payload;
  payload.reserve(frame_size);
  auto num_data_stripes = compute_num_data_stripes(frame_size, stripe_size_);
  for (uint16_t stripe = 0; stripe < num_data_stripes; stripe++) {
    payload.append(recovered_stripe(frame_size, frame_num, stripe));
  }
  return payload;
}

void StreamingCode::pad_frames(vector<optional<uint64_t>>
    frame_sizes, uint16_t timeslot)
{
  for (uint16_t t = 0; t < num_frames_; t++) {
    if (frame_sizes.at(t).has_value()) {
      uint16_t num_data_stripes = compute_num_data_stripes(
        frame_sizes.at(t).value(), stripe_size_);
      auto fnum = recovered_frame_pos_to_frame_num(timeslot, t);
      blockCode_->pad(fnum, num_data_stripes);
    }
  }
}

void StreamingCode::decode(vector<optional<uint64_t>>
    frame_sizes)
{
  assert(timeslot_.has_value());
  if (frame_sizes.size() != num_frames_) {
    throw runtime_error("Wrong number of sizes of frames\n");
  }
  pad_frames(frame_sizes, timeslot_.value());
  if (blockCode_->can_recover()) {
    blockCode_->decode();
  }
  if (blockCode_->can_recover()) {
    blockCode_->decode();
  }
}

uint16_t StreamingCode::get_stripe_size() { return stripe_size_; }
