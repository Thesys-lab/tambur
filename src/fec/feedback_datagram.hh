#ifndef FEEDBACK_DATAGRAM_HH
#define FEEDBACK_DATAGRAM_HH

#include <string>

struct FeedbackDatagram
{
  uint8_t qr;
  // size after serialization
  static constexpr size_t FEEDBACK_SIZE = sizeof(uint8_t);
  size_t feedback_size() const { return FEEDBACK_SIZE; }

  // construct this feedback by parsing binary string on wire
  bool parse_from_string(const std::string & binary);

  // serialize this feedback to binary string on wire
  std::string serialize_to_string() const;
};

#endif /* FEEDBACK_DATAGRAM_HH */
