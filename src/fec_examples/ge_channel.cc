#include "ge_channel.hh"

bool GEChannel::is_received()
{
  float drop_draw = distribution_(generator_);
  if (bad_state_) {
    return drop_draw >= p_loss_bad_;
  }
  else {
    return drop_draw >= p_loss_good_;
  }
}

void GEChannel::transition()
{
  float transition_draw = distribution_(generator_);
  if (bad_state_) {
    if (transition_draw <= p_bad_to_good_) { bad_state_ = false; }
  }
  else {
    if (transition_draw <= p_good_to_bad_) { bad_state_ = true; }
  }
}