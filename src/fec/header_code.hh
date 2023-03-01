#ifndef HEADER_CODE_HH
#define HEADER_CODE_HH

#include <string>
#include <vector>

class HeaderCode
{
public:
  // encode the current frame size to be stored in the pkt header
  virtual uint64_t encode_sizes_of_frames(uint64_t frame_size,
      uint16_t pkt_pos_in_frame) = 0;

  // computes tuple of (frame_num, size_of_frame) for decoded frame sizes
  virtual std::vector<std::pair<uint16_t, uint64_t>> decode_sizes_of_frames(
        uint16_t frame_num, uint16_t pos_in_frame, uint64_t size) = 0;
};

#endif /* HEADER_CODE_HH */
