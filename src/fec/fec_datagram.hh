#ifndef FEC_DATAGRAM_HH
#define FEC_DATAGRAM_HH

#include <string>

struct FECDatagram
{
  // header
  uint32_t seq_num; // sequence number
  bool is_parity; // serialized to 1 bit: parity or data packet
  uint16_t frame_num; // serialized to 15 bits: frame number
  // systematic code of frame size and (possibly) sizes of prior tau frames
  uint64_t sizes_of_frames_encoding;
  uint16_t pos_in_frame; // position of packet in frame
  uint16_t stripe_pos_in_frame; // position of packet in frame

  // hack in case super large frame split
  static constexpr size_t VIDEO_FRAME_INFO_SIZE = (sizeof(uint32_t) +
      2 * sizeof(uint8_t));

  std::string payload;

  // header size after serialization
  static constexpr size_t HEADER_SIZE = 2 * sizeof(uint16_t) +
      + sizeof(uint64_t) + sizeof(uint8_t) + sizeof(uint32_t);
  size_t header_size() const { return HEADER_SIZE; }

  // construct this datagram by parsing binary string on wire
  bool parse_from_string(const std::string & binary, bool get_payload = true);

  bool parse_header_from_string(const std::string & binary)
  {
    return parse_from_string(binary, false);
  }

  // serialize this datagram to binary string on wire
  std::string serialize_to_string() const;
};

inline bool operator==(const FECDatagram & first, const FECDatagram & second)
{
  return first.serialize_to_string() == second.serialize_to_string();
}

#endif /* FEC_DATAGRAM_HH */
