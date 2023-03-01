#include <stdexcept>
#include <iostream>
#include <chrono>
#include <optional>
using namespace std::chrono;

#include "fec_multi_receiver.hh"
#include "timestamp.hh"
#include "fec_datagram.hh"
#include "streaming_code.hh"
#include "serialization.hh"

using namespace std;

FECMultiReceiver::FECMultiReceiver(Code * code, uint8_t tau, uint8_t memory,
      HeaderCode * headerCode, QualityReporter * qualityReporter,
      Logger * logger, uint64_t interval_ms, bool pad,
      bool video_frame_info_padded) : code_(code), tau_(tau), memory_(memory),
      headerCode_(headerCode), qualityReporter_(qualityReporter),
      logger_(logger), interval_ms_(interval_ms),
      video_frame_info_padded_(video_frame_info_padded)
{
  target_ = timestamp_ms();
  if ((memory_) > (256 - 1)) {
    throw runtime_error("Memory is too large: the maximum value is 255\n");
  }
  if (memory_ != num_frames_for_delay(tau)) {
    throw runtime_error("Memory is invalid\n");
  }
  if (pad) {
    auto stripe_size = code->get_stripe_size();
    for (uint32_t j = 0; j < num_frames_for_delay(tau); j++) {
      string frame_str;
      frame_str += put_number((uint8_t) 3);
      frame_str += put_number((uint8_t) num_frames_for_delay(tau));
      frame_str += put_number(uint32_t(2147483640));
      frame_str.append(string(stripe_size - frame_str.size(), char(0)));
      FECDatagram pad {uint32_t(j), false, uint16_t(j), uint64_t(frame_str.size()),
          uint16_t(0), uint16_t(0), frame_str};
      WireParser test (put_number(uint32_t(2147483640)));
      receive_pkt(pad.serialize_to_string());
    }
  }
}

bool FECMultiReceiver::feedback_ready() const
{
  return timestamp_ms() >= target_;
}

uint64_t FECMultiReceiver::time_remaining() const
{
  return feedback_ready()? 0 : target_ - timestamp_ms();
}

FeedbackDatagram FECMultiReceiver::get_feedback()
{
  if (!feedback_ready()) {
    throw runtime_error("Feedback is not yet unavailable\n");
  }
  target_ += interval_ms_;
  logger_->begin_function("qualityReporter_->generate_quality_report");
  auto report = qualityReporter_->generate_quality_report();
  logger_->end_function();
  logger_->log_value("qr", int(report.qr));
  return report;
}

int FECMultiReceiver::index_of_frame(uint16_t frame_num) const
{
  for (size_t i = 0; i < frames_.size(); ++i) {
    const auto& frame = frames_.at(i);
    if (frame.get_frame_num() == frame_num) { return i; }
  }
  return -1;
}

optional<vector<FECDatagram>> FECMultiReceiver::get_frame_pkts(
    uint16_t frame_num) const
{
  optional<vector<FECDatagram>> frame_pkts;
  int index = index_of_frame(frame_num);

  if (index >= 0) {
    frame_pkts = frames_.at(index).get_frame_pkts();
  }
  return frame_pkts;
}

bool FECMultiReceiver::frame_expired(uint16_t new_frame_num, uint16_t old_frame_num)
    const
{
  assert(old_frame_num <= new_frame_num);
  return ((new_frame_num - old_frame_num) >= memory_);
}

void FECMultiReceiver::purge_frames(uint16_t frame_num)
{
  uint16_t num_frames_stored = frames_.size();
  if (num_frames_stored == 0) { return; }
  while (!frames_.empty() and frame_expired(frame_num, frames_.front().get_frame_num()))
  {
    qualityReporter_->receive_frame(frames_.front());
    logger_->add_frame(frames_.front(), frames_log_.front());
    frames_.pop_front();
    frames_log_.pop_front();
  }
}

void FECMultiReceiver::place_pkt_in_frame(const FECDatagram & pkt)
{
  auto frame_num = pkt.frame_num;
  int index = index_of_frame(frame_num);
  if (index < 0) {
    add_new_frame(pkt, true);
  }
  else {
    frames_.at(index).add_pkt(pkt);
    auto& fl = frames_log_.at(index);
    auto cur_ts = timestamp_us();
    if (pkt.is_parity) {
      if (fl.first_par_ts_ == 0) {
        fl.first_par_ts_ = cur_ts;
      }
      fl.last_par_ts_ = cur_ts;
    } else {
      if (fl.first_data_ts_ == 0) {
        fl.first_data_ts_ = cur_ts;
      }
      fl.last_data_ts_ = cur_ts;
    }

    if (fl.not_decoded_ && frames_.at(index).is_decoded())
    {
      fl.not_decoded_ = false;
      fl.decoded_after_frames_ = frames_.size() - index - 1;
      fl.decode_ts_ = cur_ts;
    }
    fl.all_data_received_ = frames_.at(index).all_data_received();
  }
}

void FECMultiReceiver::add_new_frame(const FECDatagram & pkt, bool is_valid_pkt) {
  auto frame_num = pkt.frame_num;
  auto it = frames_.begin();
  auto log_it = frames_log_.begin();
  for (; it != frames_.end(); ++it, ++log_it) {
    auto pos_frame_num = it->get_frame_num();
    if ((frame_num < pos_frame_num) and ((pos_frame_num - frame_num) <= memory_-1))
    {
      break;
    }
  }

  it = frames_.emplace(it, pkt.frame_num, !is_valid_pkt);
  log_it = frames_log_.emplace(log_it, pkt.frame_num);

  if (!is_valid_pkt) {
    return;
  }

  it->add_pkt(pkt);

  auto cur_ts = timestamp_us();

  log_it->first_ts_ = cur_ts;
  log_it->all_data_received_ = it->all_data_received();
  if (pkt.is_parity) {
    log_it->first_par_ts_ = cur_ts;
    log_it->last_par_ts_ = cur_ts;
  } else {
    log_it->first_data_ts_ = cur_ts;
    log_it->last_data_ts_ = cur_ts;
  }
  if (log_it->not_decoded_ && it->is_decoded())
  {
    log_it->not_decoded_ = false;
    log_it->decoded_after_frames_ = std::distance(it, frames_.end()) - 1;
    log_it->decode_ts_ = cur_ts;
  }
}

void FECMultiReceiver::update_frame_sizes(
      const vector<pair<uint16_t, uint64_t>> & frame_sizes_and_valid)
{
   for (uint16_t index = 0; index < frame_sizes_and_valid.size(); index++) {
    auto& frame_info = frame_sizes_and_valid.at(index);
    for (auto it = frames_.begin(); it != frames_.end(); ++it) {
      if (it->get_frame_num() == frame_info.first) {
        it->update_size(frame_info.second);
      }
    }
  }
}

void FECMultiReceiver::update_payloads(
      const vector<uint16_t> & decode_info)
{
  auto log_it = frames_log_.begin();
  for (auto it = frames_.begin(); it != frames_.end(); ++it, ++log_it) {
    if ( (!it->is_decoded() && std::find(begin(decode_info), end(decode_info), it->get_frame_num()) != std::end(decode_info) ) || (log_it->not_decoded_ && it->is_decoded())) {
      it->set_decoded();
      log_it->not_decoded_ = false;
      log_it->decoded_after_frames_ = std::distance(it, frames_.end()) - 1;
      log_it->decode_ts_ = timestamp_us();
      if (it->get_frame_val().size() != it->get_frame_size().value()) {
        string frame = code_->recovered_frame(it->get_frame_num(), (uint16_t) it->get_frame_size().value());
        if (video_frame_info_padded_) {
          string video_frame_info = frame.substr(frame.size() -
              FECDatagram::VIDEO_FRAME_INFO_SIZE,
              FECDatagram::VIDEO_FRAME_INFO_SIZE);
          WireParser parser(video_frame_info);
          uint8_t frameType = parser.read_uint8();
          uint8_t num_frames_in_video_frame = parser.read_uint8();
          uint32_t video_frame_num = parser.read_uint32();
          it->set_video_frame_info(video_frame_num, frameType,
              num_frames_in_video_frame);
          frame.erase(frame.size() -
              FECDatagram::VIDEO_FRAME_INFO_SIZE, FECDatagram::VIDEO_FRAME_INFO_SIZE);
        };
        it->set_frame_val(frame);
      }
    }
  }
}

void FECMultiReceiver::decode_headers(FECDatagram const& pkt)
{
  const auto header_decode = headerCode_->decode_sizes_of_frames(pkt.frame_num, pkt.pos_in_frame, pkt.sizes_of_frames_encoding);
  update_frame_sizes(header_decode);
}

void FECMultiReceiver::decode_payloads(FECDatagram const& pkt)
{
  code_->add_packet(pkt, pkt.frame_num);
  std::vector<std::optional<uint64_t>> frame_sizes;
  if (frames_.size() < memory_) {
    uint16_t pad = memory_ - frames_.size();
    for (uint16_t l = 0; l < pad; l++) {
      frame_sizes.push_back(std::optional<uint64_t>());
    }
  }
  for (auto const& fr : frames_)
  {
    frame_sizes.push_back(fr.get_frame_size());
  }
  code_->decode(frame_sizes);
  vector<uint16_t> decode_info;
  for (auto& fr : frames_) {
    if (!fr.get_frame_size().has_value()) { continue; }
    if (code_->is_frame_recovered(fr.get_frame_num(), fr.get_frame_size().value())) {
      decode_info.push_back(fr.get_frame_num());
      fr.set_decoded();
      if (fr.get_frame_val().size() != fr.get_frame_size().value()) {
        string frame = code_->recovered_frame(fr.get_frame_num(), (uint16_t) fr.get_frame_size().value());
        if (video_frame_info_padded_) {
          string video_frame_info = frame.substr(frame.size() -
              FECDatagram::VIDEO_FRAME_INFO_SIZE,
              FECDatagram::VIDEO_FRAME_INFO_SIZE);
          WireParser parser(video_frame_info);
          uint8_t frameType = parser.read_uint8();
          uint8_t num_frames_in_video_frame = parser.read_uint8();
          uint32_t video_frame_num = parser.read_uint32();
          fr.set_video_frame_info(video_frame_num, frameType,
              num_frames_in_video_frame);
          frame.erase(frame.size() -
              FECDatagram::VIDEO_FRAME_INFO_SIZE, FECDatagram::VIDEO_FRAME_INFO_SIZE);
        };
        fr.set_frame_val(frame);
      }
    }
  }
  update_payloads(decode_info);
}

void FECMultiReceiver::terminate()
{
  // Purge all frames (except last tau_) for proper logging
  while (frames_.size() > tau_)
  {
    qualityReporter_->receive_frame(frames_.front());
    frames_.front().clear();
    logger_->add_frame(frames_.front(), frames_log_.front());
    frames_.pop_front();
    frames_log_.pop_front();
  }
}

bool FECMultiReceiver::receive_pkt(const string & binary_datagram)
{
  logger_->begin_function("receive_pkt");
  FECDatagram pkt;
  pkt.parse_from_string(binary_datagram);
  if (pkt.payload == "[SENTINEL]") {
    terminate();
    std::cerr << "received [SENTINEL]" << std::endl;
    return false;
  }
  auto frame_num = pkt.frame_num;
  fill_missing_frames(pkt);
  purge_frames(frame_num);
  place_pkt_in_frame(pkt);
  assert(frames_.size() <= memory_);
  if (pkt.frame_num >= memory_-1){
    assert(frames_.size() >= memory_);
  }
  decode_headers(pkt);
  decode_payloads(pkt);
  logger_->end_function();
  received_first_frame_ = true;
  last_received_frame_ = pkt.frame_num;
  assert(frames_log_.back().frame_num_ == pkt.frame_num);
  assert(frames_.back().get_frame_num() == pkt.frame_num);
  frames_log_.back().all_data_received_ = frames_.back().all_data_received();
  return true;
}

optional<bool> FECMultiReceiver::is_frame_recovered(uint16_t frame_num)
{
  optional<bool> recovered;
  auto index = index_of_frame(frame_num);
  if (index < 0 or !frames_.at(index).get_frame_size().has_value()) {
    recovered = false;
    return recovered;
  }
  recovered = code_->is_frame_recovered(frame_num, frames_.at(index).get_frame_size().value());
  assert(recovered.value() == frames_.at(index).is_decoded());
  return recovered;
}

std::string FECMultiReceiver::recovered_frame(uint16_t frame_num)
{
  assert(is_frame_recovered(frame_num).value());
  int index = index_of_frame(frame_num);
  assert(index >= 0);
  return frames_.at(index).get_frame_val();
}

void FECMultiReceiver::fill_missing_frames(const FECDatagram& pkt)
{
  if ( received_first_frame_ && (pkt.frame_num == last_received_frame_ || pkt.frame_num == last_received_frame_+1)) {
    return;
  }
  uint16_t start_frame_num = received_first_frame_ ? last_received_frame_ + 1 : 0;
  while (start_frame_num != pkt.frame_num) {
    FECDatagram dummy_pkt{uint16_t(0), false, start_frame_num, 0, uint8_t(0), uint8_t(0), ""};
    add_new_frame(dummy_pkt, false);
    start_frame_num++;
  }
}

bool FECMultiReceiver::video_frame_info_available(uint16_t frame_num)
{
  int index = index_of_frame(frame_num);
  assert(index >= 0);
  return frames_.at(index).video_frame_info_available();
}

tuple<uint16_t, uint8_t, uint8_t> FECMultiReceiver::video_frame_info(
    uint16_t frame_num)
{
  assert(video_frame_info_available(frame_num));
  int index = index_of_frame(frame_num);
  assert(index >= 0);
  return frames_.at(index).video_frame_info();
}

vector<pair<uint16_t, uint32_t>> FECMultiReceiver::recovered_video_frames()
{
  vector<pair<uint16_t, uint32_t>> recovered_frames;
  recovered_frames.reserve(frames_.size());
  for (auto f : frames_) {
    if (f.is_decoded()) {
      assert(f.video_frame_info_available());
      recovered_frames.push_back({ f.get_frame_num(), f.get_video_frame_number()});
    }
  }
  return recovered_frames;
}
