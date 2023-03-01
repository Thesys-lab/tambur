#ifndef WRAP_HELPERS_HH
#define WRAP_HELPERS_HH
#include <cinttypes>
#include<deque>
#include "frame.hh"

bool frames_are_out_of_order(const std::deque<Frame> & frames);
bool order_is_switched(uint16_t next_frame_num, uint16_t previous_frame_num);

#endif /* WRAP_HELPERS_HH */
