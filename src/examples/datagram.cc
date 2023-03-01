#include <cassert>
#include <stdexcept>

#include "datagram.hh"
#include "serialization.hh"

using namespace std;

bool Datagram::parse_from_string(const string & binary)
{
  if (binary.size() < header_size()) {
    return false; // datagram is too small to contain a header
  }

  WireParser parser(binary);
  header.seq_num = parser.read_uint32();
  header.send_ts = parser.read_uint64();
  payload = parser.read_string();

  return true;
}

string Datagram::serialize_to_string() const
{
  string binary;
  binary.reserve(header_size() + payload.size());

  binary += put_number(header.seq_num);
  binary += put_number(header.send_ts);
  binary += payload;

  return binary;
}
