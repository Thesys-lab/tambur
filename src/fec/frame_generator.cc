#include <cassert>

#include "frame_generator.hh"
#include "serialization.hh"
using namespace std;

FrameGenerator::FrameGenerator(Code * code, uint8_t tau,
    Packetization * packetization, HeaderCode * headerCode, Logger * logger,
    int seed) : code_(code), packetization_(packetization),
    headerCode_(headerCode), logger_(logger)
{
  tau_ = tau;
  frame_sizes_.reserve(tau + 1);
  for (uint16_t index = 0; index < tau + 1; index++) {
    frame_sizes_.push_back(0);
  }
  frame_num_ = 0;
  sqn_ = 0;
  generator_.seed(seed);
  for (int i = 0; i < 256; i++) {
    chars_[i] = (char)i;
  }
  state_ = 0;
}

string FrameGenerator::random_str(uint64_t size)
{
  string random_str;
  random_str.reserve(size);
  uniform_int_distribution<int> rand_char(0,255);
  for (uint64_t i = 0; i < size; i++) {
    random_str += chars_[rand_char(generator_)];
  }
  return random_str;
}

vector<FECDatagram> FrameGenerator::generate_frame_data_pkts(
      const vector<pair<uint16_t, bool>> & frame_pkts_info, uint16_t size,
      uint8_t * frame, uint32_t video_frame_num, uint8_t frameType,
      uint8_t num_frames_in_video_frame)
{
  vector<FECDatagram> data_pkts;
  uint8_t num_data_pkts = 0;
  for (uint8_t index = 0; index < frame_pkts_info.size(); index++) {
    num_data_pkts += !frame_pkts_info.at(index).second;
  }
  data_pkts.reserve(num_data_pkts);
  auto stripe_size = code_->get_stripe_size();
  uint8_t stripe_pos = 0;
  string frame_str;
  bool set_frame = frame != nullptr;
  if (set_frame) {
    auto padded_frame_size = (size / stripe_size + ((size % stripe_size) > 0)) * stripe_size;
    frame_str.reserve(padded_frame_size);
    frame_str = string(frame, frame + size -
        FECDatagram::VIDEO_FRAME_INFO_SIZE);
    frame_str += put_number(frameType);
    frame_str += put_number(num_frames_in_video_frame);
    frame_str += put_number(video_frame_num);
    assert(frame_str.size() == size);
  }
  auto datagram_size = frame_pkts_info.front().first;
  for (uint8_t index = 0; index < frame_pkts_info.size(); index++) {
    if (frame_pkts_info.at(index).second) {
      continue;
    }
    if ((index < (num_data_pkts - 1)) or (0 == (size %
        datagram_size))) {
      const FECDatagram pkt {sqn_++, false, frame_num_,
          headerCode_->encode_sizes_of_frames(frame_sizes_.back(), index),
          index, stripe_pos, set_frame?
          frame_str.substr(index * datagram_size, datagram_size) :
              random_str(datagram_size)};
      data_pkts.push_back(pkt);
    }
    else {
      uint16_t pkt_size = size % datagram_size;
      uint16_t padding_size = datagram_size - pkt_size;
      vector<char> padding(padding_size, char(0));
      string pad;
      pad.reserve(padding.size());
      for (auto p : padding) { pad.push_back(p); }
      const FECDatagram pkt {sqn_++, false, frame_num_,
          headerCode_->encode_sizes_of_frames(frame_sizes_.back(), index),
          index, stripe_pos, (set_frame?  frame_str.substr(index * datagram_size,
              pkt_size) : random_str(pkt_size) ).append(pad)};
      data_pkts.push_back(pkt);
    }
    stripe_pos += data_pkts.back().payload.size() / stripe_size;
  }
  return data_pkts;
}

vector<FECDatagram> FrameGenerator::generate_frame_parity_pkts(
      const std::vector<FECDatagram> & data_pkts,
      const vector<pair<uint16_t, bool>> & frame_pkts_info,
      uint16_t frame_size)
{
  logger_->begin_function("generate_frame_parity_pkts");
  vector<FECDatagram> parity_pkts;
  uint8_t num_parity_pkts = frame_pkts_info.size() - data_pkts.size();
  vector<uint16_t> parity_pkt_sizes;
  parity_pkt_sizes.reserve(num_parity_pkts);
  auto stripe_size = code_->get_stripe_size();
  uint8_t stripe_pos = 0;
  for (uint8_t index = 0; index < frame_pkts_info.size(); index++) {
    const auto pkt_info = frame_pkts_info.at(index);
    if (pkt_info.second) {
      parity_pkt_sizes.push_back(pkt_info.first);
    }
  }
  parity_pkts.reserve(num_parity_pkts);
  logger_->begin_function("encode");
  const vector<string> payloads = code_->encode(data_pkts,
      parity_pkt_sizes, frame_size);
  logger_->end_function();
  for (uint8_t index = 0; index < num_parity_pkts; index++) {
    const FECDatagram pkt {sqn_++, true, frame_num_,
        headerCode_->encode_sizes_of_frames(frame_sizes_.back(),
        index + data_pkts.size()), index + data_pkts.size(),
        stripe_pos, payloads.at(index)};
    parity_pkts.push_back(pkt);
    stripe_pos += pkt.payload.size() / stripe_size;
  }
  logger_->end_function();
  return parity_pkts;
}

void FrameGenerator::update_frame_sizes(uint16_t frame_size)
{
  for (uint8_t index = 0; index < tau_; index++) {
    frame_sizes_[index] = frame_sizes_[index + 1];
  }
  frame_sizes_[tau_] = frame_size;
}

vector<FECDatagram> FrameGenerator::generate_frame_pkts(uint64_t frame_size, uint32_t video_frame_num, uint8_t frameType,
    uint8_t num_frames_in_video_frame, uint8_t * frame)
{
  logger_->begin_function("generate_frame_pkts");
  update_frame_sizes(frame_size);
  const auto frame_pkts_info = packetization_->get_packetization(frame_size,
      state_);
  vector<FECDatagram> pkts;
  pkts.reserve(frame_pkts_info.size());
  const auto data_pkts = generate_frame_data_pkts(frame_pkts_info, frame_size,
      frame, video_frame_num, frameType,
      num_frames_in_video_frame);
  const auto parity_pkts = generate_frame_parity_pkts(data_pkts, frame_pkts_info,
      frame_size);
  uint8_t data_index = 0;
  uint8_t parity_index = 0;
  for (uint8_t index = 0; index < frame_pkts_info.size(); index++) {
    if (frame_pkts_info.at(index).second) {
      pkts.push_back(parity_pkts.at(parity_index++));
    }
    else {
      pkts.push_back(data_pkts.at(data_index++));
    }
  }
  frame_num_++;
  logger_->end_function();
  return pkts;
}
