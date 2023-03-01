#ifndef LOSS_COMPUTER_HH
#define LOSS_COMPUTER_HH

#include <deque>

#include "frame.hh"
#include "loss_info.hh"

class LossComputer
{
public:
  LossComputer(std::deque<Frame> const & frames, bool pre_fec = true);

  LossInfo get_loss_info() { return lossInfo_; }

private:
  bool pre_fec_{true};
  LossInfo lossInfo_;

  void clear();

  void empty_frame(const Frame & frame, bool pre_fec);

  void nonempty_frame(const Frame & frame, bool pre_fec);

  std::vector<bool> get_frame_packet_losses(const Frame & frame, bool pre_fec);
};

#endif /* LOSS_COMPUTER_HH */
