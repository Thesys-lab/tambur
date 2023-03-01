#ifndef HELPERS_HH
#define HELPERS_HH
#include "reed_solomon_multi_frame_packetization.hh"
#include "streaming_code.hh"
#include "multi_fec_header_code.hh"
#include "frame_generator.hh"
#include "frame.hh"
#include "fec_sender.hh"
#include "frame_logger.hh"

std::vector<bool> get_missing(uint8_t num_pkts, uint8_t num_missing);

uint16_t get_highest_missing_pos(const std::vector<bool> & missing);

Logger get_Logger();

Packetization * get_Packetization(uint16_t tau, uint16_t stripe_size, uint16_t w);

FrameGenerator get_FrameGenerator(uint16_t tau = 0, uint16_t seed = 0);

Frame get_Frame(uint16_t frame_num = 0, bool is_dummy = false);

FECSender get_FECSender(uint16_t tau = 0);

std::vector<uint64_t> get_frame_sizes(uint16_t num_frames = 1000);

bool frame_expired(uint16_t new_frame_num, uint16_t old_frame_num, uint16_t memory);

#endif /* HELPERS_HH */
