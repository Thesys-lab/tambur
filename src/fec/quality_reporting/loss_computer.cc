#include <algorithm>
#include <stdexcept>
#include <cassert>

#include "loss_computer.hh"
#include "wrap_helpers.hh"

using namespace std;

LossComputer::LossComputer(std::deque<Frame> const & frames, bool pre_fec ) {
  clear();
  for (auto const& f : frames) {
    lossInfo_.indices.push_back(lossInfo_.packet_losses.size());
    if (!f.has_pkts()) {
      empty_frame(f, pre_fec);
    } else {
      nonempty_frame(f, pre_fec);
    }
  }
}

void LossComputer::clear()
{
  lossInfo_.indices.clear();
  lossInfo_.frame_losses.clear();
  lossInfo_.packet_losses.clear();
}

void LossComputer::empty_frame(const Frame & frame, bool pre_fec)
{
  lossInfo_.frame_losses.push_back(pre_fec? true : !frame.is_decoded());
  lossInfo_.packet_losses.push_back(true);
}

void LossComputer::nonempty_frame(const Frame & frame, bool pre_fec)
{
  lossInfo_.frame_losses.push_back(!frame.is_decoded());
  vector<bool> frame_pkt_losses = get_frame_packet_losses(frame, pre_fec);
  for (auto loss : frame_pkt_losses) {
    lossInfo_.packet_losses.push_back(loss);
    if (pre_fec and loss) { lossInfo_.frame_losses.back() = true; }
  }
}

vector<bool> LossComputer::get_frame_packet_losses(const Frame & frame, bool
    pre_fec)
{
  vector<bool> packet_losses;
  auto all_losses = frame.get_frame_packet_losses();
  for (uint16_t i = 0; i < all_losses.size(); i++) {
    bool loss = all_losses.at(i);
    if (!pre_fec) {
      loss |= frame.is_decoded() && !frame.get_frame_pkts()[i].is_parity;
    }
    packet_losses.push_back(loss);
  }
  return packet_losses;
}