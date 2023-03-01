#ifndef REED_SOLOMON_MULTI_FRAME_PACKETIZATION_HH
#define REED_SOLOMON_MULTI_FRAME_PACKETIZATION_HH

#include <deque>
#include <optional>

#include "../streaming_code/streaming_code_packetization.hh"

class ReedSolomonMultiFramePacketization : public Packetization
{
public:
  ReedSolomonMultiFramePacketization(uint16_t stripe_size, std::map<int, std::pair<int, int>>
      qr_to_parity_per_data, uint16_t block_size = 1, uint16_t w = 8,
      uint16_t max_pkt_size = Packetization::MTU);
  std::vector<std::pair<uint16_t, bool>> get_packetization(uint16_t size,
      int state) override;

  uint16_t count_sz(bool is_parity, std::vector<std::pair<uint16_t, bool>> & pkts);

private:
  uint16_t block_size_;
  uint16_t max_pkt_size_;
  uint16_t frame_num_;
  uint16_t w_;
  uint16_t stripe_size_;
  std::deque<uint16_t> parities_per_frame_;
  std::optional<StreamingCodePacketization> streamingCodePacketization_;
  std::vector<std::pair<uint16_t, bool>> previous_pkts_;
  std::vector<std::pair<uint16_t, bool>> end_of_block(
      std::vector<std::pair<uint16_t, bool>> frame_pkts);
};

#endif /* REED_SOLOMON_MULTI_FRAME_PACKETIZATION_HH */
