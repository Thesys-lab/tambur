#ifndef DATAGRAM_HH
#define DATAGRAM_HH

#include <string>

struct Datagram
{
  struct Header {
    uint32_t seq_num; // sequence number
    uint64_t send_ts; // timestamp (ms) when the datagram is sent
  };

  Header header;
  std::string payload;

  // header size after serialization
  static constexpr size_t HEADER_SIZE = sizeof(uint32_t) + sizeof(uint64_t);
  size_t header_size() const { return HEADER_SIZE; }

  // construct this datagram by parsing binary string on wire
  bool parse_from_string(const std::string & binary);

  // serialize this datagram to binary string on wire
  std::string serialize_to_string() const;
};

#endif /* DATAGRAM_HH */
