#ifndef STREAMING_CODE_HH
#define STREAMING_CODE_HH

#include <tuple>
#include <optional>
#include <string>

#include "code.hh"
#include "matrix.hh"
#include "block_code.hh"
#include "streaming_code_auxiliary_functions.hh"

using namespace std;

class StreamingCode : public Code
{
public:
  StreamingCode(uint16_t delay, uint16_t stripe_size, BlockCode* blockCode, uint16_t w = 8, uint16_t
      frame_max_data_stripes = 48, uint16_t frame_max_fec_stripes = 15);

  // creates payloads for parity packets
  virtual std::vector<std::string> encode(
      const std::vector<FECDatagram> & data_pkts,
      const std::vector<uint16_t> & parity_pkt_sizes, uint16_t frame_size) override;

  //called whenever a packet is recieved only non-expired frames
  virtual void add_packet(const FECDatagram & pkt, uint16_t frame_num) override;

  virtual void decode(std::vector<std::optional<uint64_t>>
      frame_sizes) override;

  virtual bool is_frame_recovered(uint16_t frame_num, uint16_t frame_size)
      override;

  virtual std::string recovered_frame(uint16_t frame_num, uint16_t frame_size)
      override;

  virtual bool is_stripe_recovered(uint16_t frame_num, uint16_t stripe_num, uint16_t frame_size)
      override;

  virtual std::string recovered_stripe(uint16_t frame_size, uint16_t frame_num, uint16_t stripe_num)
      override;

  virtual uint16_t get_stripe_size() override;

  uint16_t updated_ts(uint16_t frame_num);

  void pad_frames(std::vector<std::optional<uint64_t>>
    frame_sizes, uint16_t timeslot);

  void set_timeslot(uint16_t timeslot) { timeslot_ = timeslot; }

  uint16_t recovered_frame_pos_to_frame_num(uint16_t timeslot, uint16_t pos)
  {
    return (timeslot + pos + 1) % num_frames_;
  }

private:
  uint16_t delay_;
  uint16_t num_frames_;
  uint16_t stripe_size_;
  uint16_t frame_max_data_stripes_;
  uint16_t frame_max_fec_stripes_;
  std::optional<uint16_t> timeslot_;
  CodingMatrixInfo codingMatrixInfo_;
  BlockCode* blockCode_;
  std::vector<char> stripes_;
  std::vector<bool> filled_positions_;
  std::vector<int> pkt_size_of_each_frame_;
};

#endif /* STREAMING_CODE_HH */
