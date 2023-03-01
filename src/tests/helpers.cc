#include <random>
#include <cassert>

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <iterator>
#include <string>
#include <vector>

#include "helpers.hh"

using namespace std;

vector<bool> get_missing(uint8_t num_pkts, uint8_t num_missing)
{
  vector<bool> is_missing(num_pkts, false);
  assert(num_missing <= num_pkts);
  uint8_t num_dropped = 0;
  while (num_dropped < num_missing)
  {
    uint8_t index = rand() % num_pkts;
    if (!is_missing.at(index))
    {
      is_missing.at(index) = true;
      num_dropped++;
    }
  }
  return is_missing;
}

uint16_t get_highest_missing_pos(const std::vector<bool> &missing)
{
  uint16_t highest_pos = 0;
  for (uint16_t ind = 0; ind < missing.size(); ind++)
  {
    if (!missing.at(ind))
    {
      highest_pos = ind;
    }
  }
  return highest_pos;
}

Logger get_Logger()
{
  static vector<MetricLogger *> metricLoggers;
  Logger logger("tests/", metricLoggers);
  return logger;
}

Packetization *get_Packetization(uint16_t tau, uint16_t stripe_size, uint16_t w)
{
  Packetization *packetization = new ReedSolomonMultiFramePacketization(stripe_size, map<int, pair<int, int>>{{0, {0, 1}}, {1, {1, 2}}}, tau + 1, w, Packetization::MTU);
  return packetization;
}

FrameGenerator get_FrameGenerator(uint16_t tau, uint16_t seed)
{
  uint16_t w = 32;
  uint16_t packet_size = 8;
  uint16_t stripe_size = packet_size * w;
  uint16_t max_data_stripes_per_frame = 100;
  uint16_t max_fec_stripes_per_frame = 50;
  uint16_t num_frames = num_frames_for_delay(tau);
  uint16_t n_cols = num_frames;
  uint16_t n_rows = 256;
  while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
  static CodingMatrixInfo codingMatrixInfoHeader { n_rows, n_cols, 8};
  static CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(tau) *
      max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * max_data_stripes_per_frame), w};
  static Packetization * packetizationSender = get_Packetization(tau,
      stripe_size, w);
  static BlockCode blockCodeHeaderSender(codingMatrixInfoHeader, tau,
      pair<uint16_t, uint16_t>{1, 0 }, 8, false);
  static MultiFECHeaderCode headerCodeSender(&blockCodeHeaderSender, tau,
      codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));
  static BlockCode blockCodeFECSender(codingMatrixInfoFEC, tau,
      pair<uint16_t, uint16_t>{1, 0 }, packet_size, false);
  static StreamingCode codeSender(tau, stripe_size,  &blockCodeFECSender, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);
  static Logger logger = get_Logger();
  FrameGenerator frameGenerator(&codeSender, tau, packetizationSender,
                                &headerCodeSender, &logger, seed);
  return frameGenerator;
}

Frame get_Frame(uint16_t frame_num, bool is_dummy)
{
  Frame frame(frame_num, is_dummy);
  return frame;
}

FECSender get_FECSender(uint16_t tau)
{
  static FrameGenerator frameGenerator = get_FrameGenerator(tau);
  static Logger logger = get_Logger();
  FECSender fECSender(frameGenerator, tau, &logger, 1);
  return fECSender;
}

vector<uint64_t> get_frame_sizes(uint16_t num_frames)
{
  vector<uint64_t> frame_sizes;
  frame_sizes.reserve(num_frames);
  for (uint16_t index = 0; index < num_frames; index++)
  {
    frame_sizes.push_back(10500 + (index % 50));
  }
  return frame_sizes;
}

bool frame_expired(uint16_t new_frame_num, uint16_t old_frame_num, uint16_t memory)
{
  bool expired;
  if (old_frame_num <= new_frame_num)
  {
    expired = ((new_frame_num - old_frame_num) >= memory);
  }
  else if ((((65536 / 2 - 1) - old_frame_num) > memory) or
           (new_frame_num > memory))
  {
    expired = (old_frame_num - new_frame_num) > 10 * (memory + 2) * (memory + 2);
  }
  else
  {
    auto diff_above = ((65536 / 2 - 1) - old_frame_num) + 1;
    expired = (diff_above + new_frame_num) > memory;
  }
  return expired;
}
