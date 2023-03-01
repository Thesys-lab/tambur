#ifndef STREAMING_CODE_AUXILIARY_FUNCTIONS_HH
#define STREAMING_CODE_AUXILIARY_FUNCTIONS_HH

#include <tuple>
#include <optional>
#include <string>
#include <vector>

#include "fec_datagram.hh"
#include "block_code.hh"

bool final_data_stripe_of_frame_diff_size(uint16_t frame_size, uint16_t stripe_size,
    uint16_t relative_stripe_of_frame, uint16_t num_data_stripes);

uint16_t final_pos_within_stripe(uint16_t frame_size, uint16_t stripe_size,
    uint16_t relative_stripe_of_frame);

uint16_t compute_num_data_stripes(uint16_t frame_size, uint16_t stripe_size);

uint16_t first_pos_of_frame_in_block_code(uint16_t frame, uint16_t num_frames,
    uint16_t max_data_stripes, uint16_t max_fec_stripes, bool is_parity);

uint16_t first_pos_of_frame_in_block_code_erased(uint16_t frame, uint16_t num_frames,
    uint16_t max_data_stripes, uint16_t max_fec_stripes, bool is_parity);

std::string get_frame(const std::vector<FECDatagram> & data_pkts,
    uint16_t frame_size);

std::vector<char> get_stripe(const std::string & frame, uint16_t pos,
    uint16_t relative_stripe_size, uint16_t stripe_size);

void place_frame(const std::string & frame, uint16_t stripe_size, uint16_t frame_num,
    uint16_t frame_max_data_stripes, uint16_t frame_max_fec_stripes, uint16_t num_frames,
    BlockCode & blockCode, uint16_t frame_size);

std::vector<std::string> parity_payloads(CodingMatrixInfo codingMatrixInfo,
    uint16_t frame_num, const std::vector<uint16_t> & parity_pkt_sizes,
    uint16_t stripe_size, uint16_t frame_max_fec_stripes, BlockCode & blockCode,
    uint16_t delay);

std::string get_all_payloads(CodingMatrixInfo codingMatrixInfo, uint16_t total,
    uint16_t frame_num, uint16_t stripe_size, BlockCode & blockCode,
    uint16_t delay);

std::vector<std::string> set_payloads(CodingMatrixInfo codingMatrixInfo,
    std::vector<std::string> & payloads, uint16_t total, uint16_t packet_size,
    uint16_t frame_num, uint16_t stripe_size, BlockCode & blockCode, uint16_t
    delay);

std::vector<std::string> reserve_parity_payloads(const std::vector<uint16_t>
    & parity_pkt_sizes, uint16_t packet_size);

bool expired_frame(uint16_t frame_num, uint16_t timeslot, uint16_t num_frames);

std::pair<uint16_t, uint16_t> cols_of_frame_for_size(uint16_t frame_num,
    uint16_t size, uint16_t num_frames, uint16_t stripe_size, uint16_t
    frame_max_data_stripes, uint16_t frame_max_fec_stripes);

#endif /* STREAMING_CODE_AUXILIARY_FUNCTIONS_HH */
