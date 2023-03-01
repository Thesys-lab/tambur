#ifndef MULTI_FEC_HEADER_CODE_HH
#define MULTI_FEC_HEADER_CODE_HH

#include "header_code.hh"
#include "multi_frame_fec_helpers.hh"
#include "block_code.hh"

class MultiFECHeaderCode : public HeaderCode {
  public:
    MultiFECHeaderCode(BlockCode * blockCode, uint16_t delay, uint16_t
        max_pkts_per_frame);

    uint64_t encode_sizes_of_frames(uint64_t frame_size,
        uint16_t pkt_pos_in_frame ) override;

    std::vector<std::pair<uint16_t, uint64_t>> decode_sizes_of_frames(
        uint16_t frame_num, uint16_t pos_in_frame, uint64_t size) override;

  private:
    int timeslot_;
    uint16_t delay_;
    uint16_t num_frames_;
    uint16_t max_pkts_per_frame_;
    CodingMatrixInfo codingMatrixInfo_;
    BlockCode * blockCode_;
    std::vector<std::pair<uint16_t, uint64_t>> recover();
};

#endif /* MULTI_FEC_HEADER_CODE_HH */
