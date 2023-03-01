#ifndef FRAME_GENERATOR_HH
#define FRAME_GENERATOR_HH

#include <random>

#include "packetization.hh"
#include "fec_datagram.hh"
#include "header_code.hh"
#include "code.hh"
#include "logger.hh"

class FrameGenerator
{
public:
  FrameGenerator(Code * code, uint8_t tau, Packetization * packetization,
      HeaderCode * headerCode, Logger * logger, int seed);

  // creates the data packets and parity packets for the frame
  std::vector<FECDatagram> generate_frame_pkts(uint64_t frame_size,
      uint32_t video_frame_num, uint8_t frameType,
      uint8_t num_frames_in_video_frame, uint8_t * frame = nullptr);

  // updates the state for packetization (i.e., how much redundancy to use)
  void update_state(int new_state) {
    state_ = new_state;
  }

private:
  Code * code_;
  uint8_t tau_;
  std::vector<uint64_t> frame_sizes_;
  Packetization * packetization_;
  HeaderCode * headerCode_;
  Logger * logger_;
  char chars_[256];
  uint16_t frame_num_;
  uint32_t sqn_;
  std::default_random_engine generator_;
  int state_;

  // creates a string of size random bytes
  std::string random_str(uint64_t size);

  // creates the data packets for the next frame using frame_size random bytes
  std::vector<FECDatagram> generate_frame_data_pkts(
      const std::vector<std::pair<uint16_t, bool>> & frame_pkts_info, uint16_t
      frame_size, uint8_t * frame, uint32_t video_frame_num,
      uint8_t frameType, uint8_t num_frames_in_video_frame);

  // creates the parity packets based on data pkts and packetization
  std::vector<FECDatagram> generate_frame_parity_pkts(
      const std::vector<FECDatagram> & data_pkts,
      const std::vector<std::pair<uint16_t, bool>> & frame_pkts_info,
      uint16_t frame_size);

  // updates the vector of the sizes of the most recent tau + 1 frames
  void update_frame_sizes(uint16_t frame_size);

};

#endif /* FRAME_GENERATOR_HH */
