#include <tuple>
#include <cmath>
#include <cassert>

#include "frame.hh"

using namespace std;

vector<bool> Frame::get_packet_losses() const
{
  return is_missing_;
}

FECDatagram Frame::empty_pkt(std::string debug, bool is_parity)
{
  return FECDatagram {uint32_t(first_sqn_.value() + uint32_t(pkts_.size())),
      is_parity, frame_num_, size_.has_value() ? size_.value() : 0,
      uint16_t(pkts_.size()), uint16_t(pkts_.size()), debug};
}

void Frame::pad_for_pkt_pos(uint16_t packet_pos, bool is_parity)
{
  while (packet_pos > pkts_.size()) {
    pkts_.push_back(empty_pkt("pad", is_parity));
    is_missing_.push_back(true);
  }
}

void Frame::add_pkt(const FECDatagram & pkt)
{
  first_sqn_ = pkt.seq_num - uint32_t(pkt.pos_in_frame);
  last_sqn_ = pkt.seq_num;
  pad_for_pkt_pos(pkt.pos_in_frame, pkts_.size() ? pkts_.back().is_parity : false);
  pkts_.push_back(pkt);
  is_missing_.push_back(false);
}

void Frame::set_decoded()
{
  is_decoded_ = true;
}

bool Frame::is_decoded() const
{
  if (is_decoded_) return true;
  return all_data_received();
}

bool Frame::all_data_received() const
{
  if (!size_.has_value()) { return false; }
  uint64_t size_received = 0;
  for (uint16_t index = 0; index < pkts_.size(); index++) {
    const auto pkt = pkts_.at(index);
    if (!is_missing_.at(index) and !pkt.is_parity) {
      size_received += uint64_t(pkt.payload.size());
    }
  }
  return size_received >= size_.value();
}

tuple<uint32_t, uint8_t, uint8_t> Frame::video_frame_info()
{
  assert(video_frame_info_available());
  return make_tuple(video_frame_num_, frameType_, num_frames_in_video_frame_);
}

void Frame::set_video_frame_info(uint32_t video_frame_num, uint8_t frameType,
    uint8_t num_frames_in_video_frame)
{
  video_frame_info_available_ = true;
  video_frame_num_ = video_frame_num;
  frameType_ = frameType;
  num_frames_in_video_frame_ = num_frames_in_video_frame;
}
