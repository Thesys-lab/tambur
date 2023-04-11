// Modified from https://github.com/microsoft/ringmaster/blob/main/src/app/decoder.cc
#include <sys/sysinfo.h>
#include <cassert>
#include <iostream>
#include <stdexcept>
#include <algorithm>

#include "decoder.hh"
#include "exception.hh"
#include "conversion.hh"
#include "image.hh"
#include "timestamp.hh"

using namespace std;
using namespace chrono;

VideoFrame::VideoFrame(const uint32_t frame_id,
                       const FrameType frame_type,
                       const uint16_t frag_cnt)
    : id_(frame_id), type_(frame_type), frags_(frag_cnt), null_frags_(frag_cnt)
{
  if (frag_cnt == 0) {
    throw runtime_error("frame cannot have zero fragments");
  }
}

void VideoFrame::set_type_and_frags(FrameType frame_type, uint16_t frag_cnt)
{
  type_ = frame_type;
  if (frags_.size() > frag_cnt)
  {
    null_frags_ -= (frags_.size() - frag_cnt);
    frags_.erase(frags_.begin() + frag_cnt, frags_.end());
  }
}

bool VideoFrame::has_frag(const uint16_t frag_id) const
{
  return frags_.at(frag_id).has_value();
}

Datagram & VideoFrame::get_frag(const uint16_t frag_id)
{
  return frags_.at(frag_id).value();
}

const Datagram & VideoFrame::get_frag(const uint16_t frag_id) const
{
  return frags_.at(frag_id).value();
}

bool VideoFrame::complete() const
{
  return (num_fec_frames_ > 0) and (num_fec_frames_ == fec_frames_.size());
}


optional<size_t> VideoFrame::frame_size() const
{
  if (not complete()) {
    return nullopt;
  }
  optional<size_t> frame_size = 0;
  for (auto const &[key, val] : fec_frames_)
  {
    frame_size.value() += val.size();
  }
  return frame_size;
}

string VideoFrame::get_frame()
{
  assert(complete());
  if (frame_val_.size() > 0)
  {
    return frame_val_;
  }
  size_t the_frame_size = frame_size().value();
  frame_val_.reserve(the_frame_size);
  size_t last_key = 0;
  for (auto const &[key, val] : fec_frames_)
  {
    assert(last_key <= key);
    last_key = key;
    frame_val_.append(val);
  }
  return frame_val_;
}

void VideoFrame::validate_datagram(const Datagram & datagram) const
{
  if (datagram.frame_id != id_ or
      datagram.frame_type != type_ or
      datagram.frag_id >= frags_.size() or
      datagram.frag_cnt != frags_.size()) {
    throw runtime_error("unable to insert an incompatible datagram");
  }
}

void VideoFrame::insert_frag(const Datagram & datagram)
{
  validate_datagram(datagram);

  // insert only if the datagram does not exist yet
  if (not frags_[datagram.frag_id]) {
    frame_size_ += datagram.payload.size();
    null_frags_--;
    frags_[datagram.frag_id] = datagram;
  }
}

void VideoFrame::insert_frag(Datagram && datagram)
{
  validate_datagram(datagram);

  // insert only if the datagram does not exist yet
  if (not frags_[datagram.frag_id]) {
    frame_size_ += datagram.payload.size();
    null_frags_--;
    frags_[datagram.frag_id] = move(datagram);
  }
}

void VideoFrame::add_fec_frame(uint16_t fec_frame_num, FrameType frameType,
                               uint8_t num_fec_frames, string &frame)
{
  if (not fec_frames_.count(fec_frame_num))
  {
    fec_frames_[fec_frame_num] = move(frame);
  }
  type_ = frameType;
  num_fec_frames_ = num_fec_frames;
}

Decoder::Decoder(const uint16_t display_width,
                 const uint16_t display_height,
                 const int lazy_level,
                 const string &output_path,
                 FECMultiReceiver *fECMultiReceiver)
    : display_width_(display_width), display_height_(display_height),
      lazy_level_(), output_fd_consume_(), output_fd_decode_(), decoder_epoch_(steady_clock::now()),
      fECMultiReceiver_(fECMultiReceiver)
{
  // validate lazy level
  if (lazy_level < DECODE_DISPLAY or lazy_level > NO_DECODE_DISPLAY) {
    throw runtime_error("Invalid lazy level: " + to_string(lazy_level));
  }
  lazy_level_ = static_cast<LazyLevel>(lazy_level);

  // both main and worker threads start from the same time for stats output
  last_stats_time_ = decoder_epoch_;

  // open the output file
  if (not output_path.empty())
  {
    string pth = output_path;
    pth.append("_decode.csv");
    output_fd_decode_ = FileDescriptor(check_syscall(
        open(pth.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644)));
  }
  if (not output_path.empty())
  {
    string pth = output_path;
    pth.append("_consume.csv");
    output_fd_consume_ = FileDescriptor(check_syscall(
        open(pth.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644)));
  }

  // start the worker thread only if we are going to decode or display frames
  if (lazy_level <= DECODE_ONLY) {
    worker_ = thread(&Decoder::worker_main, this);
    cerr << "Spawned a new thread for decoding and displaying frames" << endl;
  }
}

void Decoder::terminate()
{
  fECMultiReceiver_->terminate(); 
  terminated_ = true;

  if (lazy_level_ != NO_DECODE_DISPLAY) { 
    if (vpx_codec_destroy(&context_) != VPX_CODEC_OK)
    {
      cerr << "Failed to destroy VPX decoder context" << endl;
    }
  }
}

void Decoder::pad_dummy_frames(uint32_t frame_id)
{
  for (uint32_t id = next_frame_; id < frame_id; id++)
  {
    if (not frame_buf_.count(id)) {
      frame_buf_.emplace(piecewise_construct,
                       forward_as_tuple(id),
                       forward_as_tuple(id, FrameType::DUMMY, 65535));
    }
  }
}

bool Decoder::add_datagram_common(const Datagram & datagram)
{
  const auto frame_id = datagram.frame_id;
  const auto frame_type = datagram.frame_type;
  const auto frag_cnt = datagram.frag_cnt;

  // ignore any datagrams from the old frames
  if (frame_id < next_frame_) {
    return false;
  }

  if (not frame_buf_.count(frame_id)) {
    pad_dummy_frames(frame_id);
    // initialize a Frame instance for frame 'frame_id'
    frame_buf_.emplace(piecewise_construct,
                       forward_as_tuple(frame_id),
                       forward_as_tuple(frame_id, frame_type, frag_cnt));
  }
  frame_buf_.at(datagram.frame_id).set_type_and_frags(frame_type, frag_cnt);
  return true;
}

bool Decoder::add_datagram(const Datagram & datagram)
{
  if (not add_datagram_common(datagram)) {
    return false;
  }
  fECMultiReceiver_->receive_pkt(datagram.payload);
  // copy the fragment into the frame
  frame_buf_.at(datagram.frame_id).insert_frag(datagram);
  update_fec_recovered_frames();
  return true;
}

bool Decoder::add_datagram(Datagram && datagram)
{
  if (not add_datagram_common(datagram)) {
    return false;
  }
  fECMultiReceiver_->receive_pkt(datagram.payload);
  // move the fragment into the frame
  frame_buf_.at(datagram.frame_id).insert_frag(move(datagram));
  update_fec_recovered_frames();
  return true;
}

void Decoder::update_fec_recovered_frames()
{
  vector<pair<uint16_t, uint32_t>> fec_video_recovered_frames =
      fECMultiReceiver_->recovered_video_frames();
  for (auto el : fec_video_recovered_frames)
  {
    auto fec_frame = el.first;
    auto video_frame = el.second;
    if (video_frame < next_frame_ or (fec_frame < fECMultiReceiver_->get_num_frames()))
    {
      continue;
    }
    auto video_frame_info = fECMultiReceiver_->video_frame_info(fec_frame);
    if (not frame_buf_.count(video_frame)) {
      pad_dummy_frames(video_frame+1);
    }
    VideoFrame &frame = frame_buf_.at(video_frame);
    if (not frame.has_fec_frame(fec_frame))
    {
      string s = fECMultiReceiver_->recovered_frame(fec_frame);
      frame.add_fec_frame(fec_frame, FrameType{get<1>(video_frame_info)},
                          get<2>(video_frame_info), s);
    }
  }
}

bool Decoder::next_frame_complete()
{
  {
    // check if the next frame to expect is complete
    auto it = as_const(frame_buf_).find(next_frame_);
    if (it != frame_buf_.end() and it->second.complete()) {
      return true;
    }
  }

  // seek forward if a key frame in the future is already complete
  for (auto it = frame_buf_.rbegin(); it != frame_buf_.rend(); it++) {
    const auto frame_id = it->first;
    const auto & frame = it->second;

    // found a complete key frame ahead of next_frame_
    if (frame.type() == FrameType::KEY and frame.complete()) {
      assert(frame_id > next_frame_);

      // set next_frame_ to frame_id and clean up old frames
      const auto frame_diff = frame_id - next_frame_;
      advance_next_frame(frame_diff);

      cerr << "* Recovery: skipped " << frame_diff
           << " frames ahead to key frame " << frame_id << endl;

      return true;
    }
  }

  return false;
}

void Decoder::consume_next_frame()
{
  VideoFrame &frame = frame_buf_.at(next_frame_);
  if (not frame.complete()) {
    throw runtime_error("next frame must be complete before consuming it");
  }

  // found a decodable frame; update (and output) stats
  num_decodable_frames_++;
  const size_t frame_size = frame.frame_size().value();
  total_decodable_frame_size_ += frame_size;

  const auto stats_now = steady_clock::now();
  while (stats_now >= last_stats_time_ + 1s) {
    cerr << "Decodable frames in the last ~1s: "
         << num_decodable_frames_ << endl;

    const double diff_ms = duration<double, milli>(
                           stats_now - last_stats_time_).count();
    if (diff_ms > 0) {
      cerr << "  - Bitrate (kbps): "
           << double_to_string(total_decodable_frame_size_ * 8 / diff_ms)
           << endl;
    }

    // reset stats
    num_decodable_frames_ = 0;
    total_decodable_frame_size_ = 0;
    last_stats_time_ += 1s;
  }

  if (lazy_level_ <= DECODE_ONLY) {
    // dispatch the frame to worker thread
    {
      lock_guard<mutex> lock(mtx_);
      shared_queue_.emplace_back(move(frame));
    } // release the lock before notifying the worker thread

    // notify worker thread
    cv_.notify_one();
  } else {
    // main thread outputs frame information if no worker thread
    if (output_fd_consume_) {
      const auto frame_decodable_ts = timestamp_us();

      output_fd_consume_->write(to_string(next_frame_) + "," +
                        to_string(frame_size) + "," +
                        to_string(frame_decodable_ts) + "\n");
    }
  }

  // move onto the next frame
  advance_next_frame();
}

void Decoder::advance_next_frame(const unsigned int n)
{
  next_frame_ += n;

  // clean up state up to next_frame_
  clean_up_to(next_frame_);
}

void Decoder::clean_up_to(const uint32_t frontier)
{
  deque<uint32_t> to_clean;
  for (auto it = frame_buf_.begin(); it != frame_buf_.end(); ++it)
  {
    if (it->first < frontier)
    {
      to_clean.emplace_back(it->first);
    }
  }
  while (to_clean.size() > 0)
  {
    if (acked_frames_.count(to_clean.front()))
    {
      acked_frames_.erase(to_clean.front());
    }
    frame_buf_.erase(to_clean.front());
    to_clean.pop_front();
  }
}

double Decoder::decode_frame(vpx_codec_ctx_t & context, VideoFrame & frame)
{
  if (not frame.complete()) {
    throw runtime_error("frame must be complete before decoding");
  }

  // allocate a decoding buffer once
  static constexpr size_t MAX_DECODING_BUF = 1000000; // 1 MB
  static vector<uint8_t> decode_buf(MAX_DECODING_BUF);

  // copy the payload of the frame's datagrams to 'decode_buf'
  uint8_t * buf_ptr = decode_buf.data();
  const uint8_t * const buf_end = buf_ptr + decode_buf.size();
  const string &frame_val = frame.get_frame();
  if (buf_ptr + frame_val.size() >= buf_end)
  {
    throw runtime_error("frame size exceeds max decoding buffer size");
  }
  memcpy(buf_ptr, frame_val.data(), frame_val.size());

  const size_t frame_size = frame_val.size();

  // decode the compressed frame in 'decode_buf'
  const auto decode_start = steady_clock::now();
  check_call(vpx_codec_decode(&context, decode_buf.data(), frame_size,
                              nullptr, 1),
             VPX_CODEC_OK, "failed to decode a frame");
  const auto decode_end = steady_clock::now();

  return duration<double, milli>(decode_end - decode_start).count();
}

void Decoder::display_decoded_frame(vpx_codec_ctx_t & context,
                                    VideoDisplay & display)
{
  vpx_codec_iter_t iter = nullptr;
  vpx_image * raw_img;
  unsigned int frame_decoded = 0;

  // display the decoded frame stored in 'context_'
  while ((raw_img = vpx_codec_get_frame(&context, &iter))) {
    frame_decoded++;

    // there should be exactly one frame decoded
    if (frame_decoded > 1) {
      throw runtime_error("Multiple frames were decoded at once");
    }

    // construct a temporary RawImage that does not own the raw_img
    display.show_frame(RawImage(raw_img));
  }
}

void Decoder::worker_main()
{
  // worker does nothing if not decode or display
  if (lazy_level_ == NO_DECODE_DISPLAY) {
    return;
  }

  // initialize a VP9 decoding context
  const unsigned int max_threads = min(get_nprocs(), 16);
  cerr << "FEC encoding and decoding with tambur each requires an extra thread; ensure these two threads plus cpu_used in encoder.cc plus max_threads is less than the total number of threads available"<< endl;
  vpx_codec_dec_cfg_t cfg {max_threads, display_width_, display_height_};

  check_call(vpx_codec_dec_init(&context_, &vpx_codec_vp9_dx_algo, &cfg, 0),
             VPX_CODEC_OK, "vpx_codec_dec_init");

  cerr << "[worker] Initialized decoder (max threads: "
       << max_threads << ")" << endl;

  // video display
  unique_ptr<VideoDisplay> display;
  if (lazy_level_ == DECODE_DISPLAY) {
    display = make_unique<VideoDisplay>(display_width_, display_height_);
  }

  // local queue of frames
  deque<VideoFrame> local_queue;

  // stats maintained by the worker thread
  unsigned int num_decoded_frames = 0;
  double total_decode_time_ms = 0.0;
  double max_decode_time_ms = 0.0;
  auto last_stats_time = decoder_epoch_;

  while (not terminated_) {
    // destroy display if it has been signalled to quit
    if (display and display->signal_quit()) {
      display.reset(nullptr);
    }

    {
      // wait until the shared queue is not empty
      unique_lock<mutex> lock(mtx_);
      cv_.wait(lock, [this] { return (not shared_queue_.empty()) and (not terminated_); });

      // worker owns the lock after wait and should copy shared queue quickly
      while (not shared_queue_.empty()) {
        local_queue.emplace_back(move(shared_queue_.front()));
        shared_queue_.pop_front();
      }
    } // worker releases the lock so it doesn't block the main thread anymore

    // now worker can take its time to decode and render the frames kept locally
    while (not local_queue.empty()) {
      VideoFrame & frame = local_queue.front();
      const double decode_time_ms = decode_frame(context_, frame);

      if (output_fd_consume_) {
        const auto frame_decoded_ts = timestamp_us();

        output_fd_consume_->write(to_string(frame.id()) + "," +
                          to_string(frame.frame_size().value()) + "," +
                          to_string(frame_decoded_ts) + "\n");
      }

      if (display) {
        display_decoded_frame(context_, *display);
      }

      local_queue.pop_front();

      // update stats
      num_decoded_frames++;
      total_decode_time_ms += decode_time_ms;
      max_decode_time_ms = max(max_decode_time_ms, decode_time_ms);
      
      // worker thread also outputs stats roughly every second
      const auto stats_now = steady_clock::now();
      while (stats_now >= last_stats_time + 1s) {
        if (num_decoded_frames > 0) {
          cerr << "[worker] Avg/Max decoding time (ms) of "
               << num_decoded_frames << " frames: "
               << double_to_string(total_decode_time_ms / num_decoded_frames)
               << "/" << double_to_string(max_decode_time_ms) << endl;
        }

        // reset stats
        num_decoded_frames = 0;
        total_decode_time_ms = 0.0;
        max_decode_time_ms = 0.0;
        last_stats_time += 1s;
      }
    }
  }

  return;
}

vector<FRAMEMsg> Decoder::decoded_unacked_frames()
{
  vector<FRAMEMsg> decoded;
  decoded.reserve(frame_buf_.size());
  for (auto it = frame_buf_.rbegin(); it != frame_buf_.rend(); it++)
  {
    const auto frame_id = it->first;
    const auto &frame = it->second;
    if (frame.complete() and (not acked_frames_.count(frame_id)))
    {
      decoded.push_back(FRAMEMsg{frame_id});
      if (output_fd_decode_)
      {
        const auto frame_decodable_ts = timestamp_us();
        output_fd_decode_->write(to_string(frame_id) + "," +
                          to_string(frame_decodable_ts) + "\n");
      }
    }
  }
  return decoded;
}

void Decoder::acked(uint32_t frame_id)
{
  acked_frames_.insert({frame_id, true});
}
