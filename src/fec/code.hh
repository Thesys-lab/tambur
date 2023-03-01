#ifndef CODE_HH
#define CODE_HH

#include <stdexcept>
#include <string>
#include <vector>
#include<optional>

#include "fec_datagram.hh"

class Code
{
public:
  // creates payloads for parity packets
  virtual std::vector<std::string> encode(
      const std::vector<FECDatagram> & data_pkts,
      const std::vector<uint16_t> & parity_pkt_sizes, uint16_t frame_size) = 0;

 //called whenever a packet is recieved only non-expired frames
  virtual void add_packet(const FECDatagram & pkt, uint16_t frame_num) = 0;

  virtual void decode(std::vector<std::optional<uint64_t>>
      frame_sizes) = 0;

  virtual bool is_frame_recovered(uint16_t frame_num, uint16_t frame_size) = 0;

  virtual std::string recovered_frame(uint16_t frame_num, uint16_t frame_size)
      = 0;

  virtual bool is_stripe_recovered(uint16_t frame_num, uint16_t stripe_num, uint16_t frame_size) = 0;

  virtual std::string recovered_stripe(uint16_t frame_size, uint16_t frame_num, uint16_t stripe_num) = 0;

  virtual uint16_t get_stripe_size() = 0;
};

#endif /* CODE_HH */
