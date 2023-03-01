#ifndef FEC_SENDER_HH
#define FEC_SENDER_HH

#include <stdexcept>

#include "frame_generator.hh"
#include "feedback_datagram.hh"
#include "logger.hh"
#include "multi_frame_fec_helpers.hh"

class FECSender
{
public:
  FECSender(FrameGenerator frameGenerator, uint8_t tau, Logger * logger,
      uint64_t interval_ms = 30, uint16_t max_frame_size_possible = 65535, bool pad = false);


  // determines if it is time to generate the next frame
  bool frame_ready() const;

  // see how much time is left before the next frame should be generated
  uint64_t time_remaining() const;

  // generates the next frame
  std::vector<FECDatagram> next_frame(uint16_t frame_size, bool new_time = true);

  // generates packts for frame
  std::vector<FECDatagram> next_frame(uint16_t frame_size, uint8_t * frame,
      uint32_t video_frame_num, uint8_t frameType,
      uint8_t num_frames_in_video_frame);

  // updates state based on feedback from receiver
  void handle_feedback(const std::string & binary_feedback);

  int get_state() { return state_; }

  uint16_t max_frame_size() { return max_frame_size_possible_; }

private:
  FrameGenerator frameGenerator_;
  uint8_t tau_;
  Logger * logger_;
  uint64_t interval_ms_;
  int state_;
  uint64_t target_;
  uint16_t max_frame_size_possible_;

};

#endif /* FEC_SENDER_HH */
