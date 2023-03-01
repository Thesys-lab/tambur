#ifndef FRAME_HH
#define FRAME_HH

#include <vector>
#include <optional>
#include <iostream>
#include <cassert>

#include "packetization.hh"
#include "fec_datagram.hh"

class Frame
{
public:
  Frame(uint16_t frame_num, bool is_dummy) : frame_num_(frame_num),
      is_dummy_(is_dummy), video_frame_info_available_(false) {}

  bool has_pkts() const
  {
    return !is_dummy_ && pkts_.size() > 0;
  }

  std::vector<bool> get_packet_losses() const;

  void add_pkt(const FECDatagram &pkt);

  void update_size(uint64_t size) { size_ = size; }

  bool is_decoded() const;

  void set_decoded();

  bool all_data_received() const;

  std::vector<FECDatagram> get_frame_pkts() const { return pkts_; }

  std::vector<bool> get_frame_packet_losses() const { return is_missing_; }

  uint16_t get_frame_num() const { return frame_num_; }

  std::optional<uint64_t> get_frame_size() const { return size_; }

  std::optional<uint32_t> get_first_sqn() const { return first_sqn_; }

  std::optional<uint32_t> get_last_sqn() const { return last_sqn_; }

  void set_video_frame_info(uint32_t video_frame_num, uint8_t frameType,
      uint8_t num_frames_in_video_frame);

  bool video_frame_info_available() { return video_frame_info_available_; }

  std::tuple<uint32_t, uint8_t, uint8_t> video_frame_info();

  uint32_t get_video_frame_number() {
    assert(video_frame_info_available());
    return video_frame_num_;
  }

  void set_frame_val(std::string & frame_val)
  {
    frame_val_ = move(frame_val);
  }

  void clear() { frame_val_.clear(); }

  std::string get_frame_val() {
    assert(is_decoded_);
    return frame_val_;
  }

private:
  uint16_t frame_num_;
  std::vector<FECDatagram> pkts_;
  std::vector<uint16_t> pkt_sizes_;
  std::vector<bool> is_missing_;
  std::optional<uint64_t> size_;
  std::optional<uint32_t> first_sqn_;
  std::optional<uint32_t> last_sqn_;
  bool is_dummy_;
  bool is_decoded_{false};
  uint32_t video_frame_num_;
  uint8_t frameType_;
  uint8_t num_frames_in_video_frame_;
  bool video_frame_info_available_;
  std::string frame_val_;

  void pad_for_pkt_pos(uint16_t packet_pos, bool is_parity);

  FECDatagram empty_pkt(std::string s, bool is_parity = true);

};

#endif /* FRAME_HH */
