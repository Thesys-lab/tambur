#ifndef STREAMING_PACKETIZATION_HH
#define STREAMING_PACKETIZATION_HH

#include <optional>
#include <string>
#include <map>
#include <deque>

#include "packetization.hh"

class StreamingCodePacketization : public Packetization
{
public:
  StreamingCodePacketization(uint16_t w, uint16_t stripe_size, std::map<int, std::pair<int, int>> qr_to_parity_per_data, uint16_t
      max_pkt_size = uint16_t(Packetization::MTU), uint16_t parity_delay = 0 );
  std::vector<std::pair<uint16_t, bool>> get_packetization(uint16_t size,
      int state) override;

  std::vector<std::pair<uint16_t, bool>> get_pkts_given_stripes(uint16_t
      num_parity_stripes, uint16_t num_data_stripes);

  uint16_t get_num_parity_stripes(uint16_t num_data_stripes, int state);

private:
  uint16_t w_;
  uint16_t stripe_size_;
  std::map<int, std::pair<int, int>> qr_to_parity_per_data_;
  uint16_t max_pkt_size_;
  uint16_t max_stripes_per_pkt_;
  uint16_t parity_delay_;
  std::deque<uint16_t> parity_stripes_;
};

#endif /* STREAMING_PACKETIZATION_HH */
