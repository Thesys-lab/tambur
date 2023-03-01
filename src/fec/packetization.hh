#ifndef PACKETIZATION_HH
#define PACKETIZATION_HH

#include <stdexcept>

#include <vector>
#include <utility>
#include <string>

class Packetization
{
public:
  // returns a vector of (size, is_parity) for packets of the frame
  virtual std::vector<std::pair<uint16_t, bool>> get_packetization(
      uint16_t size, int state) = 0;

  static constexpr size_t MTU = 1500;// max bytes per pkt under packetization
};

#endif /* PACKETIZATION_HH */
