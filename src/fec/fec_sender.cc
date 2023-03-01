#include <cassert>

#include "fec_sender.hh"
#include "timestamp.hh"

using namespace std;

FECSender::FECSender(FrameGenerator frameGenerator, uint8_t tau ,
    Logger * logger, uint64_t interval_ms, uint16_t max_frame_size_possible,
    bool pad) : frameGenerator_(frameGenerator), tau_(tau), logger_(logger),
    interval_ms_(interval_ms), state_(0), max_frame_size_possible_(max_frame_size_possible)
{
  target_ = timestamp_ms();
  if (pad) {
    uint8_t padding = uint8_t(char(0));
    for (uint16_t j = 0; j < num_frames_for_delay(tau); j++) {
      next_frame(0, &padding, uint32_t(2147483640), (uint8_t) 3,
          (uint8_t) num_frames_for_delay(tau));
    }
  }
}

bool FECSender::frame_ready() const
{
  return timestamp_ms() >= target_;
}

uint64_t FECSender::time_remaining() const
{
  return frame_ready()? 0 : target_ - timestamp_ms();
}

vector<FECDatagram> FECSender::next_frame(uint16_t frame_size, bool new_time)
{
  assert(frame_size <= max_frame_size_possible_);
  logger_->begin_function("next_frame");
  if (!frame_ready()) {
    throw runtime_error("Frame is not yet available\n");
  }
  if (new_time) {
    target_ += interval_ms_;
  }
  const auto pkts = frameGenerator_.generate_frame_pkts(frame_size, 0, 0, 0,
      nullptr);
  logger_->end_function();
  return pkts;
}



vector<FECDatagram> FECSender::next_frame(uint16_t frame_size, uint8_t * frame,
    uint32_t video_frame_num, uint8_t frameType,
    uint8_t num_frames_in_video_frame)
{
  uint16_t padded_frame_size = frame_size + FECDatagram::VIDEO_FRAME_INFO_SIZE;
  assert(padded_frame_size <= max_frame_size_possible_);
  logger_->begin_function("next_frame");
  const auto pkts = frameGenerator_.generate_frame_pkts(padded_frame_size,
      video_frame_num, frameType, num_frames_in_video_frame, frame);
  logger_->end_function();
  return pkts;
}

void FECSender::handle_feedback(const std::string & binary_feedback)
{
  logger_->begin_function("handle_feedback");
  FeedbackDatagram feedbackDatagram;
  feedbackDatagram.parse_from_string(binary_feedback);
  state_ = int(feedbackDatagram.qr);
  frameGenerator_.update_state(state_);
  logger_->end_function();
}
