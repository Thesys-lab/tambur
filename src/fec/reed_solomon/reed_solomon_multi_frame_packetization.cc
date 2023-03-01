#include <iostream>

#include "reed_solomon_multi_frame_packetization.hh"

using namespace std;

ReedSolomonMultiFramePacketization::ReedSolomonMultiFramePacketization(uint16_t
    stripe_size, map<int, pair<int, int>> qr_to_parity_per_data, uint16_t block_size,
    uint16_t w, uint16_t max_pkt_size) : block_size_(block_size),
    max_pkt_size_(max_pkt_size), frame_num_(0), w_(w), stripe_size_(stripe_size)
{
  if (size_t(max_pkt_size) > Packetization::MTU) {
    throw runtime_error("Too large max_pkt_size" + to_string(max_pkt_size) + "\n");
  }
  streamingCodePacketization_.emplace(w, stripe_size_, qr_to_parity_per_data,
      max_pkt_size);
}

vector<pair<uint16_t, bool>> ReedSolomonMultiFramePacketization::
    get_packetization(uint16_t size, int state)
{
  vector<pair<uint16_t, bool>> pkts = streamingCodePacketization_->
    get_packetization(size, state);
  vector<pair<uint16_t, bool>> frame_pkts;
  for (auto pkt : pkts) {
    previous_pkts_.push_back(pkt);
    if (!pkt.second) { frame_pkts.push_back(pkt); }
  }
  auto parity = count_sz(true, pkts);
  parities_per_frame_.emplace_back(parity);
  while (parities_per_frame_.size() > block_size_) {
    parities_per_frame_.pop_front();
  }
  if ((frame_num_++ % block_size_) == (block_size_ - 1)) {
    frame_pkts = end_of_block(frame_pkts);
    previous_pkts_.clear();
    parities_per_frame_.clear();
  }

  return frame_pkts;
}

uint16_t ReedSolomonMultiFramePacketization::count_sz(bool is_parity,
    vector<pair<uint16_t, bool>> & pkts)
{
  uint16_t count = 0;
  for (auto pkt : pkts) { if (pkt.second == is_parity) { count += pkt.first; } }
  return count;
}

vector<pair<uint16_t, bool>> ReedSolomonMultiFramePacketization::end_of_block(
    vector<pair<uint16_t, bool>> frame_pkts)
{
  uint16_t num_parity_symbols = 0;
  for (auto sz : parities_per_frame_) {
    num_parity_symbols += sz;
  }
  auto num_parity_stripes = num_parity_symbols / stripe_size_;
  auto num_data_symbols = count_sz(false, frame_pkts);
  auto num_data_stripes = (num_data_symbols / stripe_size_);
  auto pkts = streamingCodePacketization_->get_pkts_given_stripes(
    num_parity_stripes, num_data_stripes);
  return pkts;
}
