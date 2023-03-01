#include "gtest/gtest.h"
#include <random>
#include <algorithm>
#include <chrono>

#include "helpers.hh"
#include "loss_computer.hh"

using namespace std;

uint64_t MTU = (uint64_t) Packetization::MTU;
const std::vector<uint64_t> size_increment = {1, 1, 1, 1, 1, MTU - 2};

TEST(test_loss_computer, test_one_frame)
{
  srand(0);
  deque<Frame> frames0;
  deque<Frame> frames1;
  FrameGenerator frameGenerator = get_FrameGenerator(0);
  for (uint16_t state = 0; state < 2; state += 1) {
    frameGenerator.update_state(int(state));
    for (uint64_t size = MTU - 2, increment = 0; size < 10*MTU;
      size += size_increment.at(increment++ % 6)) {
      const auto pkts = frameGenerator.generate_frame_pkts(size, 0, 0, 1);
      auto num_pkts = pkts.size();
      for (uint16_t num_lost = 0; num_lost < num_pkts; num_lost++) {
        frames0.clear();
        frames1.clear();
        auto frame = get_Frame(pkts.at(0).frame_num);
        auto missing = get_missing(num_pkts, num_lost);
        for (uint16_t i = 0; i < num_pkts; i++) {
          if (not missing.at(i)) {
            auto pkt = pkts.at(i);
            frame.add_pkt(pkt);
          }
        }
        frames0.emplace_back(frame);
        LossInfo lossInfo0 = LossComputer(frames0).get_loss_info();
        uint16_t highest_pos = get_highest_missing_pos(missing);
        EXPECT_EQ(lossInfo0.indices, vector<uint16_t> {0});
        EXPECT_EQ(lossInfo0.frame_losses,  vector<bool> { true } );
        EXPECT_EQ(lossInfo0.packet_losses,
            vector<bool>(missing.begin(), missing.begin() + highest_pos+1));
        frame.update_size(size);
        frames1.emplace_back(frame);
        bool data_lost = false;
        for (uint16_t ind = 0; ind < missing.size(); ind++) {
          if (missing.at(ind) and !pkts.at(ind).is_parity) {
            data_lost = true;
          }
        }
        LossInfo lossInfoPostFEC = LossComputer(frames1, false).get_loss_info();
        EXPECT_EQ(lossInfoPostFEC.frame_losses,  vector<bool> { data_lost } );
        LossInfo lossInfo1 = LossComputer(frames1, true).get_loss_info();
        EXPECT_EQ(lossInfo1.packet_losses.size(), highest_pos + 1);
        for (uint16_t miss_pos = 0; miss_pos <= highest_pos; miss_pos++) {
          EXPECT_EQ(lossInfo1.packet_losses.at(miss_pos), missing.at(miss_pos));
        }
        EXPECT_EQ(lossInfo1.indices, vector<uint16_t> {0});
      }
    }
  }
}

TEST(test_loss_computer, test_multiple_frames)
{
  srand(0);
  deque<Frame> frames0;
  deque<Frame> frames1;
  FrameGenerator frameGenerator = get_FrameGenerator(0);
  for (uint16_t update_size = 0; update_size < 3; update_size++) {
    for (uint16_t state = 0; state < 15; state++) {
      frameGenerator.update_state(int(state % 2));
      for (uint16_t max_num_frames = 2; max_num_frames < 30; max_num_frames++) {
        frames0.clear();
        frames1.clear();
        vector<bool> packet_losses;
        vector<bool> frame_losses;
        vector<uint16_t> indices;
        for (uint16_t frame_ind = 1; frame_ind <= max_num_frames; frame_ind++) {
          uint16_t size = (rand() % MTU* (state < 1 ? 9 : 1)) + MTU - 2;
          const auto pkts = frameGenerator.generate_frame_pkts(size, 0, 0, 1);
          auto num_pkts = pkts.size();
          uint16_t num_lost = (rand() % num_pkts);
          auto frame = get_Frame(pkts.at(0).frame_num);
          auto missing = get_missing(num_pkts, num_lost);
          for (uint16_t i = 0; i < num_pkts; i++) {
            if (not missing.at(i)) {
              frame.add_pkt(pkts.at(i));
            }
          }
          if (update_size > 0) { frame.update_size(size); }
          frames0.emplace_back(frame);
          LossInfo lossInfo0 = LossComputer(frames0).get_loss_info();
          indices.push_back(packet_losses.size());
          packet_losses.insert(packet_losses.end(),
              lossInfo0.packet_losses.begin(), lossInfo0.packet_losses.end());
          uint16_t last_seen = 0;
          uint16_t first_loss = 65535;
          for (uint16_t l = 0; l < num_pkts; l++) {
            if (missing.at(l)) {
              if (l < first_loss) { first_loss = l; }
            }
            else { last_seen = l; }
          }
          bool any_data_lost = false;
          for (uint16_t l = 0; l < num_pkts; l++) {
            if (missing.at(l) and !pkts.at(l).is_parity) {
              any_data_lost = true;
            }
          }
          bool loss;
          if (update_size == 0) { loss = true; }
          else if (update_size == 1) {
            loss = (num_lost and last_seen > first_loss) or any_data_lost;
          }
          else  { loss = any_data_lost; }
          frame_losses.push_back(loss);
          frames1.emplace_back(frame);
          frames0.clear();
        }
        LossInfo lossInfo1 = LossComputer(frames1, update_size < 2).get_loss_info();
        if (update_size < 2) {
          EXPECT_EQ(packet_losses, lossInfo1.packet_losses);
        }
        EXPECT_EQ(frame_losses, lossInfo1.frame_losses);
        EXPECT_EQ(indices, lossInfo1.indices);
      }
    }
  }
}

TEST(test_loss_computer, test_missing_frame_nums)
{
  srand(0);
  deque<Frame> frames0;
  deque<Frame> frames1;
  FrameGenerator frameGenerator = get_FrameGenerator(0);
  for (uint16_t state = 0; state < 2; state += 1) {
    frameGenerator.update_state(int(state % 2));
    for (uint16_t max_num_frames = 2; max_num_frames < 30; max_num_frames++) {
      int count = 0;
      frames0.clear();
      frames1.clear();
      vector<bool> packet_losses;
      vector<bool> frame_losses;
      vector<uint16_t> indices;
      vector<Frame> frames;
      for (uint16_t frame_ind = 1; frame_ind <= max_num_frames; frame_ind++) {
        uint16_t size = (rand() % MTU* (state < 1 ? 9 : 1)) + MTU - 2;
        const auto pkts = frameGenerator.generate_frame_pkts(size, 0, 0, 1);
        auto num_pkts = pkts.size();
        uint16_t num_lost = (rand() % num_pkts);
        auto frame = get_Frame(pkts.at(0).frame_num);
        auto missing = get_missing(num_pkts, num_lost);
        if (missing.back()) { num_lost -= 1; }
        missing.back() = false;
        for (uint16_t i = 0; i < num_pkts; i++) {
          if (not missing.at(i)) {
            frame.add_pkt(pkts.at(i));
          }
        }
        frame.update_size(size);
        uint16_t next_index = packet_losses.size();
        vector<bool> next_packet_losses;
        bool next_frame_loss;
        if (frame_ind == 1 or frame_ind == max_num_frames or ((rand() % 2) == 0)) {
          frames0.emplace_back(frame);
          LossInfo lossInfo0 = LossComputer(frames0).get_loss_info();
          frames1.emplace_back(frame);
          next_packet_losses = lossInfo0.packet_losses;
          next_frame_loss = num_lost > 0;
          frames0.clear();
          count++;
        }
        else {
          next_packet_losses = vector<bool>(1, true);
          next_frame_loss = true;
          auto dummy_frame = get_Frame(pkts.at(0).frame_num, true);
          frames1.emplace_back(dummy_frame);
        }
        indices.push_back(next_index);
        frame_losses.push_back(next_frame_loss);
        packet_losses.insert(packet_losses.end(), next_packet_losses.begin(),
            next_packet_losses.end());
      }
      LossInfo lossInfo1 = LossComputer(frames1).get_loss_info();
      EXPECT_EQ(packet_losses, lossInfo1.packet_losses);
      EXPECT_EQ(frame_losses, lossInfo1.frame_losses);
      EXPECT_EQ(indices, lossInfo1.indices);
    }
  }
}

TEST(test_loss_computer, test_post_fec)
{
  srand(0);
  deque<Frame> frames;
  FrameGenerator frameGenerator = get_FrameGenerator(0);
  frameGenerator.update_state(1);
  for (uint16_t t = 0; t < 4; t += 1) {
    for (uint64_t size = MTU - 2, increment = 0; size < 3*MTU;
      size += size_increment.at(increment++ % 6)) {
      for (uint16_t num_frames = 0; num_frames <= 8*t; num_frames += t) {
        frames.clear();
        vector<bool> frame_losses;
        for (uint16_t frame_num = 0; frame_num < num_frames; frame_num++) {
          const auto pkts = frameGenerator.generate_frame_pkts(
              size + (rand() % MTU), 0, 0, 1);
          auto num_pkts = pkts.size();
          bool empty_frame = rand() % 2;
          auto frame = get_Frame(pkts.at(0).frame_num, empty_frame);
          auto missing = get_missing(num_pkts, rand() % (1 + num_pkts / 4));
          for (uint16_t i = 0; i < num_pkts; i++) {
            if (not missing.at(i) and !empty_frame) {
              auto pkt = pkts.at(i);
              frame.add_pkt(pkt);
            }
          }
          bool loss = rand() % 2;
          frame_losses.push_back(loss);
          if (!loss) { frame.set_decoded(); }
          frames.push_back(frame);
        }
        LossInfo lossInfo = LossComputer(frames, false).get_loss_info();
        EXPECT_EQ(lossInfo.frame_losses,  frame_losses );
        if (t == 0) { num_frames = 1; }
      }
    }
  }
}

int main(int argc, char * argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
