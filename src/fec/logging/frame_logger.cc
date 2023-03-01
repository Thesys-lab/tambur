#include "frame_logger.hh"

#include <iostream>

void FrameLogger::add_frame(const Frame & frame, const FrameLog & frame_log)
{
    frames_.emplace_back(frame);
    frames_log_.emplace_back(frame_log);
}

std::deque<FrameLog> FrameLogger::get_frame_logs()
{
    return frames_log_;
}