#include <cmath>
#include <cassert>
#include <vector>

#include "streaming_code_packetization.hh"

using namespace std;

StreamingCodePacketization::StreamingCodePacketization(uint16_t w,
    uint16_t stripe_size, map<int, pair<int, int>> qr_to_parity_per_data,
    uint16_t max_pkt_size, uint16_t parity_delay) : w_(w),
    stripe_size_(stripe_size), qr_to_parity_per_data_(qr_to_parity_per_data),
    max_pkt_size_(max_pkt_size), parity_delay_(parity_delay)
{
  if (size_t(max_pkt_size) > Packetization::MTU)
  {
    throw runtime_error("Too large max_pkt_size\n");
  }
  max_stripes_per_pkt_ = max_pkt_size / stripe_size;
  for (uint16_t pos = 0; pos < parity_delay_; pos++) {
    parity_stripes_.emplace_back(0);
  }
}

uint16_t StreamingCodePacketization::get_num_parity_stripes(uint16_t num_data_stripes, int state)
{
  auto data_per_parities = qr_to_parity_per_data_[state];
  auto num_parity_stripes = uint16_t(ceil((num_data_stripes * data_per_parities.first) / float(data_per_parities.second)));
  parity_stripes_.emplace_back(num_parity_stripes);
  auto n_par = parity_stripes_.front();
  parity_stripes_.pop_front();
  return n_par;
}

vector<pair<uint16_t, bool>> StreamingCodePacketization::get_packetization(
    uint16_t size, int state)
{
  auto num_data_stripes = (size / stripe_size_) + ((size % stripe_size_) > 0);
  auto num_parity_stripes = get_num_parity_stripes(num_data_stripes, state);
  return get_pkts_given_stripes(num_parity_stripes, num_data_stripes);
}

vector<pair<uint16_t, bool>> StreamingCodePacketization::get_pkts_given_stripes(
    uint16_t num_parity_stripes, uint16_t num_data_stripes)
{
  uint16_t stripes_per_pkt = max_stripes_per_pkt_;
  while ((stripes_per_pkt > 1) and ((num_parity_stripes % stripes_per_pkt) or
                                    (num_data_stripes % stripes_per_pkt)))
  {
    stripes_per_pkt--;
  }
  auto num_data_pkts = num_data_stripes / stripes_per_pkt;
  vector<pair<uint16_t, bool>> send_pkts(num_data_pkts, {stripe_size_ * stripes_per_pkt,
                                                         false});
  auto num_parity_pkts = num_parity_stripes / stripes_per_pkt;
  vector<pair<uint16_t, bool>> parity_pkts(num_parity_pkts, {stripe_size_ * stripes_per_pkt,
                                                             true});
  send_pkts.reserve(num_data_pkts + num_parity_pkts);
  for (auto pkt : parity_pkts)
  {
    send_pkts.push_back(pkt);
  }
  return send_pkts;
}
