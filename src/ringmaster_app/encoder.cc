// Modified from https://github.com/microsoft/ringmaster/blob/main/src/app/encoder.cc
#include <sys/sysinfo.h>
#include <cassert>
#include <iostream>
#include <string>
#include <stdexcept>
#include <chrono>
#include <algorithm>
#include <limits>

#include "encoder.hh"
#include "conversion.hh"
#include "timestamp.hh"
#include "serialization.hh"

using namespace std;
using namespace chrono;

Encoder::Encoder(const uint16_t display_width,
                 const uint16_t display_height,
                 const uint16_t frame_rate, const unsigned int bitrate_kbps,
                 const string &output_path,
                 FECSender *fECSender, GEChannel *gE, uint16_t tau, Logger * loggerFECSender)
    : display_width_(display_width), display_height_(display_height),
      frame_rate_(frame_rate), target_bitrate_(bitrate_kbps), output_fd_(),
      fECSender_(fECSender), gE_(gE), tau_(tau), loggerFECSender_(loggerFECSender)
{
  // open the output file
  if (not output_path.empty()) {
    output_fd_ = FileDescriptor(check_syscall(
        open(output_path.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644)));
  }

  // populate VP9 configuration with default values
  check_call(vpx_codec_enc_config_default(&vpx_codec_vp9_cx_algo, &cfg_, 0),
             VPX_CODEC_OK, "vpx_codec_enc_config_default");

  // copy the configuration below mostly from WebRTC (libvpx_vp9_encoder.cc)
  cfg_.g_w = display_width_;
  cfg_.g_h = display_height_;
  cfg_.g_timebase.num = 1;
  cfg_.g_timebase.den = frame_rate_; // WebRTC uses a 90 kHz clock
  cfg_.g_pass = VPX_RC_ONE_PASS;
  cfg_.g_lag_in_frames = 0; // disable lagged encoding
  // WebRTC disables error resilient mode unless for SVC
  cfg_.g_error_resilient = VPX_ERROR_RESILIENT_DEFAULT;
  cfg_.g_threads = 4; // encoder threads; should equal to column tiles below
  cfg_.rc_resize_allowed = 0; // WebRTC enables spatial sampling
  cfg_.rc_dropframe_thresh = 0; // WebRTC sets to 30 (% of target data buffer)
  cfg_.rc_buf_initial_sz = 500;
  cfg_.rc_buf_optimal_sz = 600;
  cfg_.rc_buf_sz = 1000;
  cfg_.rc_min_quantizer = 2;
  cfg_.rc_max_quantizer = 52;
  cfg_.rc_undershoot_pct = 50;
  cfg_.rc_overshoot_pct = 50;

  // prevent libvpx encoder from automatically placing key frames
  cfg_.kf_mode = VPX_KF_DISABLED;
  // WebRTC sets the two values below to 3000 frames (fixed keyframe interval)
  cfg_.kf_max_dist = numeric_limits<unsigned int>::max();
  cfg_.kf_min_dist = 0;

  cfg_.rc_end_usage = VPX_CBR;
  cfg_.rc_target_bitrate = target_bitrate_;

  // use no more than 16 or the number of avaialble CPUs
  const unsigned int cpu_used = min(get_nprocs(), 16);

  cerr << "FEC encoding and decoding with tambur each requires an extra thread; ensure these two threads plus cpu_used plus max_threads in decoder.cc is less than the total number of threads available"<< endl;

  // more encoder settings
  check_call(vpx_codec_enc_init(&context_, &vpx_codec_vp9_cx_algo, &cfg_, 0),
             VPX_CODEC_OK, "vpx_codec_enc_init");

  // this value affects motion estimation and *dominates* the encoding speed
  codec_control(&context_, VP8E_SET_CPUUSED, cpu_used);

  // enable encoder to skip static/low content blocks
  codec_control(&context_, VP8E_SET_STATIC_THRESHOLD, 1);

  // clamp the max bitrate of a keyframe to 900% of average per-frame bitrate
  codec_control(&context_, VP8E_SET_MAX_INTRA_BITRATE_PCT, int(100.0 * (65535.0 - float(FECDatagram::VIDEO_FRAME_INFO_SIZE)) /
                    (1000.0 * float(bitrate_kbps) / float(frame_rate))));

  // enable encoder to adaptively change QP for each segment within a frame
  codec_control(&context_, VP9E_SET_AQ_MODE, 3);

  // set the number of column tiles in encoding a frame to 2 ** 2 = 4
  codec_control(&context_, VP9E_SET_TILE_COLUMNS, 2);

  // enable row-based multi-threading
  codec_control(&context_, VP9E_SET_ROW_MT, 1);

  // disable frame parallel decoding
  codec_control(&context_, VP9E_SET_FRAME_PARALLEL_DECODING, 0);

  // enable denoiser (but not on ARM since optimization is pending)
  codec_control(&context_, VP9E_SET_NOISE_SENSITIVITY, 1);

  set_target_bitrate();

  cerr << "Initialized VP9 encoder (CPU used: " << cpu_used << ")" << endl;
}

Encoder::~Encoder()
{
  if (vpx_codec_destroy(&context_) != VPX_CODEC_OK) {
    cerr << "~Encoder(): failed to destroy VPX encoder context" << endl;
  }
}

void Encoder::compress_frame(const RawImage & raw_img)
{
  std::string const SEND_PKT_STR = "send_pkt";
  const auto frame_generation_ts = timestamp_us();

  // encode raw_img into frame 'frame_id_'
  encode_frame(raw_img);

  // packetize frame 'frame_id_' into datagrams
  const size_t frame_size = packetize_encoded_frame();

  loggerFECSender_->log_event(SEND_PKT_STR, frame_id_);
  // output frame information
  if (output_fd_) {
    const auto frame_encoded_ts = timestamp_us();

    output_fd_->write(to_string(frame_id_) + "," +
                      to_string(target_bitrate_) + "," +
                      to_string(frame_size) + "," +
                      to_string(frame_generation_ts) + "," +
                      to_string(frame_encoded_ts) + "," +
                      to_string((uint8_t)frame_type_) + "," +
                      to_string(force_keyframe_) + "," +
                      to_string(first_fec_frame_) + "," +
                      to_string(last_fec_frame_) + "," +
                      to_string(fECSender_->get_state()) + "," +
                      to_string(redundancy_size_) + "\n");
  }

  // move onto the next frame
  frame_id_++;
}

void Encoder::encode_frame(const RawImage & raw_img)
{
  if (raw_img.display_width() != display_width_ or
      raw_img.display_height() != display_height_)
  {
    throw runtime_error("Encoder: image dimensions don't match");
  }

  force_keyframe_ = false;
  // check if a key frame needs to be encoded
  vpx_enc_frame_flags_t encode_flags = 0; // normal frame
  if (not unacked_frames_.empty())
  {
    auto first_unacked = unacked_frames_.cbegin();

    // give up if first unacked datagram was initially sent MAX_UNACKED_US ago
    const auto us_since_first_send = timestamp_us() - first_unacked->second.first;

    if ((us_since_first_send > MAX_UNACKED_US) or ((highest_acked_fec_frame_.has_value() and (highest_acked_fec_frame_.value() > first_unacked->second.second)) and ((highest_acked_fec_frame_.value() - first_unacked->second.second) > tau_))) {
      encode_flags = VPX_EFLAG_FORCE_KF; // force next frame to be key frame

      cerr << "* Recovery: gave up retransmissions and forced a key frame "
           << frame_id_ << endl;

      if (verbose_)
      {
        cerr << "Giving up on lost frame: frame_id="
             << first_unacked->first
             << " us_since_first_send=" << us_since_first_send << endl;
      }

      // clean up
      send_buf_.clear();
      unacked_.clear();
      unacked_frames_.clear();
      force_keyframe_ = true;
    }
  }

  // encode a frame and calculate encoding time
  const auto encode_start = steady_clock::now();
  check_call(vpx_codec_encode(&context_, raw_img.get_vpx_image(), frame_id_, 1,
                              encode_flags, VPX_DL_REALTIME),
             VPX_CODEC_OK, "failed to encode a frame");
  const auto encode_end = steady_clock::now();
  const double encode_time_ms = duration<double, milli>(
                                encode_end - encode_start).count();

  // track stats in the current period
  num_encoded_frames_++;
  total_encode_time_ms_ += encode_time_ms;
  max_encode_time_ms_ = max(max_encode_time_ms_, encode_time_ms);
}

size_t Encoder::packetize_encoded_frame()
{
  // read the encoded frame's "encoder packets" from 'context_'
  const vpx_codec_cx_pkt_t *encoder_pkt;
  vpx_codec_iter_t iter = nullptr;
  unsigned int frames_encoded = 0;
  size_t frame_size = 0;
  gE_->transition();
  string const FRAME_SIZE_STR = "frame_size";

  while ((encoder_pkt = vpx_codec_get_cx_data(&context_, &iter))) {
    if (encoder_pkt->kind == VPX_CODEC_CX_FRAME_PKT) {
      frames_encoded++;

      // there should be exactly one frame encoded
      if (frames_encoded > 1) {
        throw runtime_error("Multiple frames were encoded at once");
      }

      frame_size = encoder_pkt->data.frame.sz;
      assert(frame_size > 0);

      // read the returned frame type
      frame_type_ = FrameType::NONKEY;
      if (encoder_pkt->data.frame.flags & VPX_FRAME_IS_KEY) {
        frame_type_ = FrameType::KEY;
        unacked_frames_.clear();
        unacked_.clear();

        if (verbose_) {
          cerr << "Encoded a key frame: frame_id=" << frame_id_ << endl;
        }
      }

      assert(fECSender_->max_frame_size() > FECDatagram::VIDEO_FRAME_INFO_SIZE);
      assert(frame_size / (fECSender_->max_frame_size() -
          FECDatagram::VIDEO_FRAME_INFO_SIZE) + 1 <= 65535);
      assert(fECSender_->max_frame_size() < (65535 - FECDatagram::VIDEO_FRAME_INFO_SIZE));

      // next address to copy compressed frame data from
      uint8_t * buf_ptr = static_cast<uint8_t *>(encoder_pkt->data.frame.buf);
      pkts_.clear();
      // split into multiple "frames" for FEC if too large
      uint64_t max_frame_size_fec = fECSender_->max_frame_size();
      uint16_t rel_num_frames = uint16_t((frame_size / max_frame_size_fec) +
                                ((frame_size % max_frame_size_fec) > 0));
      while ((frame_size / rel_num_frames +
          ((frame_size % rel_num_frames) > 0) +
          FECDatagram::VIDEO_FRAME_INFO_SIZE) > fECSender_->max_frame_size()) {
        rel_num_frames++;
      }
      uint16_t frag = 0;
      redundancy_size_ = 0;
      uint16_t num_parity = 0;
      uint64_t total_size = 0;
      for (uint64_t j = 0; j < rel_num_frames; j++)
      {
        uint16_t sz = uint16_t(frame_size / rel_num_frames) +
                      uint16_t(j < (frame_size % rel_num_frames));
        const auto pkts = fECSender_->next_frame(sz, buf_ptr,
                                                 frame_id_, (uint8_t)frame_type_, (uint8_t)rel_num_frames);
        buf_ptr += sz;
        uint16_t frame_data_size = 0;
        uint16_t frame_par_size = 0;
        for (auto pkt : pkts)
        {
          if (pkt.is_parity)
          {
            num_parity++;
            frame_par_size += pkt.payload.size();
            redundancy_size_ += pkt.payload.size();
          }
      	  else { frame_data_size += pkt.payload.size(); }
          if (gE_->is_received())
          {
            pkts_.emplace_back(make_pair(frag, move(pkt)));
          }
          frag++;
        }
      	loggerFECSender_->log_value(FRAME_SIZE_STR, 0, frame_data_size, frame_par_size, fECSender_->get_state());

        if (j == 0)
        {
          first_fec_frame_ = pkts.front().frame_num;
        }
        if (j == (rel_num_frames - 1))
        {
          last_fec_frame_ = pkts.front().frame_num;
        }
      }
      // total fragments to divide this frame into
      const uint16_t frag_cnt = pkts_.size();

      for (uint16_t pos = 0; pos < frag_cnt; pos++)
      {
        string packet = pkts_.front().second.serialize_to_string();
        auto frag_id = pkts_.front().first;
        pkts_.pop_front();
        assert(packet.size() <= Datagram::max_payload);
        // enqueue a datagram
        send_buf_.emplace_back(Datagram{frame_id_, frame_type_, frag_id,
                                        frag, string_view{packet}});
      }
    }
  }

  return frame_size;
}

void Encoder::add_unacked(const Datagram & datagram)
{
  const auto seq_num = make_pair(datagram.frame_id, datagram.frag_id);
  auto [it, success] = unacked_.emplace(seq_num, datagram);

  if (not success) {
    throw runtime_error("datagram already exists in unacked");
  }

  it->second.last_send_ts = it->second.send_ts;
}

void Encoder::add_unacked(Datagram && datagram)
{
  const auto seq_num = make_pair(datagram.frame_id, datagram.frag_id);
  auto [it, success] = unacked_.emplace(seq_num, move(datagram));

  if (not success) {
    throw runtime_error("datagram already exists in unacked");
  }

  it->second.last_send_ts = it->second.send_ts;
}

void Encoder::add_unacked_frame(uint32_t frame_id, uint16_t fec_frame_id)
{
  unacked_frames_.emplace(frame_id, make_pair(timestamp_us(), fec_frame_id));
}

void Encoder::handle_ack(const shared_ptr<AckMsg> & ack)
{
  const auto curr_ts = timestamp_us();

  // observed an RTT sample
  add_rtt_sample(curr_ts - ack->send_ts);

  update_highest_acked_fec_frame(ack->frame_id);

  // find the acked datagram in 'unacked_'
  const auto acked_seq_num = make_pair(ack->frame_id, ack->frag_id);
  auto acked_it = unacked_.find(acked_seq_num);

  if (acked_it == unacked_.end()) {
    // do nothing else if ACK is not for an unacked datagram
    return;
  }
  if (MAX_NUM_RTX > 0)
  {
    // retransmit all unacked datagrams before the acked one (backward)
    for (auto rit = make_reverse_iterator(acked_it);
        rit != unacked_.rend(); rit++) {
      auto & datagram = rit->second;

      // skip if a datagram has been retransmitted MAX_NUM_RTX times
      if (datagram.num_rtx >= MAX_NUM_RTX) {
        continue;
      }

      // retransmit if it's the first RTX or the last RTX was about one RTT ago
      if (datagram.num_rtx == 0 or
          curr_ts - datagram.last_send_ts > ewma_rtt_us_.value()) {
        datagram.num_rtx++;
        datagram.last_send_ts = curr_ts;

        // retransmissions are more urgent
        send_buf_.emplace_front(datagram);
      }
    }
  }
  // finally, erase the acked datagram from 'unacked_'
  unacked_.erase(acked_it);
}

void Encoder::handle_fec_feedback(const shared_ptr<FECMsg> &feedback)
{
  const string f = put_number(feedback->info);
  fECSender_->handle_feedback(f);
}

void Encoder::handle_frame_feedback(const shared_ptr<FRAMEMsg> &frame)
{
  auto acked_frame_id = frame->frame_id;
  update_highest_acked_fec_frame(acked_frame_id);
  auto acked_it = unacked_frames_.find(acked_frame_id);

  if (acked_it == unacked_frames_.end())
  {
    return;
  }
  unacked_frames_.erase(acked_it);
}

void Encoder::add_rtt_sample(const unsigned int rtt_us)
{
  // min RTT
  if (not min_rtt_us_ or rtt_us < *min_rtt_us_) {
    min_rtt_us_ = rtt_us;
  }

  // EWMA RTT
  if (not ewma_rtt_us_) {
    ewma_rtt_us_ = rtt_us;
  } else {
    ewma_rtt_us_ = ALPHA * rtt_us + (1 - ALPHA) * (*ewma_rtt_us_);
  }
}

void Encoder::output_periodic_stats()
{
  cerr << "Frames encoded in the last ~1s: " << num_encoded_frames_ << endl;

  if (num_encoded_frames_ > 0) {
    cerr << "  - Avg/Max encoding time (ms): "
         << double_to_string(total_encode_time_ms_ / num_encoded_frames_)
         << "/" << double_to_string(max_encode_time_ms_) << endl;
  }

  if (min_rtt_us_ and ewma_rtt_us_) {
    cerr << "  - Min/EWMA RTT (ms): " << double_to_string(*min_rtt_us_ / 1000.0)
         << "/" << double_to_string(*ewma_rtt_us_ / 1000.0) << endl;
  }

  // reset all but RTT-related stats
  num_encoded_frames_ = 0;
  total_encode_time_ms_ = 0.0;
  max_encode_time_ms_ = 0.0;
}

void Encoder::set_target_bitrate()
{
  cfg_.rc_target_bitrate = target_bitrate_;
  check_call(vpx_codec_enc_config_set(&context_, &cfg_),
             VPX_CODEC_OK, "set_target_bitrate");
}

void Encoder::update_highest_acked_fec_frame(uint32_t acked_video_frame_id)
{
  if (unacked_frames_.count(acked_video_frame_id) > 0)
  {
    uint16_t fec_frame = unacked_frames_.at(acked_video_frame_id).second;
    if (!highest_acked_fec_frame_.has_value() or
        highest_acked_fec_frame_.value() <= fec_frame)
    {
      highest_acked_fec_frame_ = fec_frame;
    }
  }
}
