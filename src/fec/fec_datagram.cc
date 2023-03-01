#include <stdexcept>

#include "fec_datagram.hh"
#include "serialization.hh"

using namespace std;

bool FECDatagram::parse_from_string(const string & binary, bool get_payload)
{
  if (binary.size() < header_size()) {
    return false; // datagram is too small to contain a header
  }

  WireParser parser(binary);
  seq_num = parser.read_uint32();

  // parse is_parity and frame_num
  uint16_t temp = parser.read_uint16();
  is_parity = get_bits(temp, 0, 1);
  frame_num = get_bits(temp, 1, 15);

  sizes_of_frames_encoding = parser.read_uint64();
  pos_in_frame = parser.read_uint16();
  stripe_pos_in_frame = parser.read_uint16();

  if (get_payload) { payload = parser.read_string(); }

  return true;
}

string FECDatagram::serialize_to_string() const
{
  string binary;
  binary.reserve(header_size() + payload.size());

  binary += put_number(seq_num);

  // serialize is_parity and frame_num
  uint16_t temp = is_parity << 15;
  if (frame_num >= (1 << 15)) {
    throw runtime_error("frame_num should fit in 15 bits");
  }
  temp += frame_num;
  binary += put_number(temp);

  binary += put_number(sizes_of_frames_encoding);
  binary += put_number(pos_in_frame);
  binary += put_number(stripe_pos_in_frame);

  binary += payload;

  return binary;
}
