#include <vector>
#include <optional>
#include <cmath>

#include "streaming_code_auxiliary_functions.hh"
#include "multi_frame_fec_helpers.hh"
#include "wrap_helpers.hh"

using namespace std;

bool final_data_stripe_of_frame_diff_size(uint16_t frame_size, uint16_t stripe_size, uint16_t relative_stripe_of_frame, uint16_t num_data_stripes)
{
  assert(relative_stripe_of_frame < num_data_stripes);
  assert(num_data_stripes * stripe_size >= frame_size);
  assert(num_data_stripes * stripe_size < frame_size + stripe_size);
  return (relative_stripe_of_frame == (num_data_stripes - 1)) and
      (frame_size % stripe_size);
}

uint16_t final_pos_within_stripe(uint16_t frame_size, uint16_t stripe_size,
    uint16_t relative_stripe_of_frame)
{
  uint16_t end_pos = stripe_size - 1;
  auto num_data_stripes = compute_num_data_stripes(frame_size, stripe_size);
  if (final_data_stripe_of_frame_diff_size(frame_size, stripe_size,
    relative_stripe_of_frame, num_data_stripes)) {
    end_pos = (frame_size % stripe_size) - 1;
  }
  return end_pos;
}

uint16_t compute_num_data_stripes(uint16_t frame_size, uint16_t stripe_size)
{
  assert(frame_size > 0);
  return frame_size / stripe_size  + ((frame_size % stripe_size) > 0);
}

uint16_t first_pos_of_frame_in_block_code(uint16_t frame, uint16_t num_frames,
    uint16_t max_data_stripes, uint16_t max_fec_stripes, bool is_parity)
{
  if (is_parity) {
    return (frame % num_frames) * max_fec_stripes;
  }
  return (frame % num_frames) * max_data_stripes;
}

uint16_t first_pos_of_frame_in_block_code_erased(uint16_t frame, uint16_t num_frames,
    uint16_t max_data_stripes, uint16_t max_fec_stripes, bool is_parity)
{
  if (is_parity) {
    return num_frames * max_data_stripes + (frame % num_frames) * max_fec_stripes;
  }
  return (frame % num_frames) * max_data_stripes;
}

bool expired_frame(uint16_t frame_num, uint16_t timeslot, uint16_t num_frames)
{
  assert(frame_num <= timeslot);
  if (timeslot < num_frames) { return false; }
  return frame_num + num_frames <= timeslot;
}

pair<uint16_t, uint16_t> cols_of_frame_for_size(uint16_t frame_num, uint16_t
    size, uint16_t num_frames, uint16_t stripe_size, uint16_t
    frame_max_data_stripes, uint16_t frame_max_fec_stripes)
{
  assert(size > 0);
  assert(size <= stripe_size * frame_max_data_stripes);
  auto first = first_pos_of_frame_in_block_code(frame_num, num_frames,
      frame_max_data_stripes, frame_max_fec_stripes, false);
  auto num_stripes_for_frame = compute_num_data_stripes(size, stripe_size);
  return { first, first + num_stripes_for_frame - 1};
}

string get_frame(const vector<FECDatagram> & data_pkts,
    uint16_t frame_size)
{
  string payload;
  if (data_pkts.size() == 0) { return payload; }
  payload.reserve(frame_size + data_pkts.front().payload.size());
  uint16_t remaining = frame_size;
  for (uint16_t ind = 0; ind < data_pkts.size(); ind++) {
    const auto pkt = data_pkts.at(ind);
    if (!pkt.is_parity) {
      const auto pkt_payload = pkt.payload;
      uint16_t pos = 0;
      while (pos < pkt_payload.size() and remaining-- != 0) {
        payload.push_back(pkt_payload.at(pos++));
      }
    }
  }
  assert(payload.size() == frame_size);
  return payload;
}

vector<char> get_stripe(const string & frame, uint16_t pos,
    uint16_t relative_stripe_size, uint16_t stripe_size)
{
  vector<char> stripe;
  uint16_t start_pos = pos * stripe_size;
  assert(start_pos < frame.size());
  assert(start_pos + relative_stripe_size <= frame.size());
  stripe.reserve(relative_stripe_size);
  for (uint16_t frame_pos = start_pos; frame_pos < start_pos +
      relative_stripe_size; frame_pos++) {
    stripe.push_back(frame.at(frame_pos));
  }
  return stripe;
}

string get_all_payloads(CodingMatrixInfo codingMatrixInfo, uint16_t total,
    uint16_t frame_num, uint16_t stripe_size, BlockCode & blockCode,
    uint16_t delay)
{
  string all_payloads;
  assert((total % stripe_size) == 0);
  all_payloads.reserve(total);
  auto endpoints = rows_of_frame(frame_num, delay, codingMatrixInfo);
  uint16_t row_pos = endpoints.first;
  while ((row_pos <= endpoints.second) and (all_payloads.size() < total)) {
    all_payloads.append(blockCode.get_row(row_pos++, true));
  }
  return all_payloads;
}

vector<string> set_payloads(CodingMatrixInfo codingMatrixInfo,
    vector<string> & payloads, uint16_t total, uint16_t packet_size,
    uint16_t frame_num, uint16_t stripe_size, BlockCode & blockCode, uint16_t delay)
{
  assert(total == packet_size * payloads.size());
  assert(total % stripe_size == 0);
  string all_payloads = get_all_payloads(codingMatrixInfo, total, frame_num,
      stripe_size, blockCode, delay);
  for (uint16_t pkt_pos = 0; pkt_pos < payloads.size(); pkt_pos++) {
    payloads.at(pkt_pos).reserve(packet_size);
    for (uint16_t index = 0; index < packet_size; index++) {
      auto nxt_char = all_payloads.at(pkt_pos * packet_size + index);
      payloads.at(pkt_pos).push_back(nxt_char);
    }
  }
  return payloads;
}

vector<string> reserve_parity_payloads(const vector<uint16_t> &
    parity_pkt_sizes, uint16_t packet_size)
{
  vector<string> payloads;
  payloads.reserve(parity_pkt_sizes.size());
  for (uint16_t pos = 0; pos < parity_pkt_sizes.size(); pos++) {
    payloads.push_back(string());
    if (parity_pkt_sizes.at(pos) != packet_size) {
      throw runtime_error("Not all same size parities\n");
    }
  }
  return payloads;
}

void place_frame(const string & frame, uint16_t stripe_size, uint16_t frame_num,
    uint16_t frame_max_data_stripes, uint16_t frame_max_fec_stripes, uint16_t num_frames,
    BlockCode & blockCode, uint16_t frame_size)
{
  uint16_t num_data_stripes = compute_num_data_stripes(frame.size(), stripe_size);
  assert(num_data_stripes <= frame_max_data_stripes);
  auto first_data_stripe = first_pos_of_frame_in_block_code(frame_num, num_frames,
      frame_max_data_stripes, frame_max_fec_stripes, false);
  for (uint16_t rel_stripe = 0; rel_stripe < num_data_stripes; rel_stripe++) {
    uint16_t rel_stripe_size = final_pos_within_stripe(frame_size, stripe_size, rel_stripe) + 1;
    vector<char> stripe = get_stripe(frame, rel_stripe, rel_stripe_size, stripe_size);
    assert(stripe.size() == rel_stripe_size);
    blockCode.place_payload(first_data_stripe + rel_stripe, false, stripe);
  }
}

vector<string> parity_payloads(CodingMatrixInfo codingMatrixInfo,
    uint16_t frame_num, const vector<uint16_t> & parity_pkt_sizes,
    uint16_t stripe_size, uint16_t frame_max_fec_stripes,
    BlockCode & blockCode, uint16_t delay)
{
  if (parity_pkt_sizes.size() == 0) { return vector<string>(); }
  uint16_t packet_size = parity_pkt_sizes.at(0);
  uint16_t total = packet_size * parity_pkt_sizes.size();
  if (total > stripe_size * frame_max_fec_stripes) {
    throw runtime_error("Invalid number of parity stripes for parity pkts\n");
  }
  vector<string> payloads = reserve_parity_payloads(parity_pkt_sizes,
      packet_size);
  return set_payloads(codingMatrixInfo, payloads, total, packet_size, frame_num,
      stripe_size, blockCode, delay);
}
