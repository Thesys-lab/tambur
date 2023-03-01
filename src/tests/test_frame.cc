#include "gtest/gtest.h"
#include <random>
#include <chrono>

#include "helpers.hh"

using namespace std;

uint64_t MTU = (uint64_t) Packetization::MTU;
const std::vector<uint64_t> size_increment = {1, 1, 1, 1, 1, MTU - 2};


TEST(test_frame, test_add_pkt_simple)
{
  FrameGenerator frameGenerator = get_FrameGenerator();
  for (uint16_t state = 0; state < 2; state += 1) {
    frameGenerator.update_state(int(state));
    for (uint64_t size = MTU - 2, increment = 0; size < 10*MTU;
        size += size_increment.at(increment++ % 6)) {
      vector<FECDatagram> pkts = frameGenerator.generate_frame_pkts(size, 0, 0, 1);
      auto frame = get_Frame();
      for (uint16_t i = 0; i < pkts.size(); i++) {
        auto pkt = pkts.at(i);
        frame.add_pkt(pkt);
        auto test_frame_pkts = frame.get_frame_pkts();
        EXPECT_EQ(test_frame_pkts.size(), i + 1);
        EXPECT_EQ(pkts.at(i), test_frame_pkts.at(i));
      }
      auto test_frame_pkts = frame.get_frame_pkts();
      for (uint16_t j = 0; j < pkts.size(); j++) {
        EXPECT_EQ(pkts.at(j), test_frame_pkts.at(j));
      }
      EXPECT_FALSE(frame.is_decoded());
      frame.update_size(size);
      EXPECT_TRUE(frame.is_decoded());
    }
  }
}

TEST(test_frame, test_any_losses_no_loss_is_decoded)
{
  srand(0);
  FrameGenerator frameGenerator = get_FrameGenerator();
  for (uint16_t trial = 0; trial < 10; trial++) {
    for (uint16_t state = 0; state < 2; state += 1) {
      frameGenerator.update_state(int(state));
      for (uint64_t size = MTU - 2, increment = 0; size < 10*MTU;
        size += size_increment.at(increment++ % 6)) {
        const auto pkts = frameGenerator.generate_frame_pkts(size, 0, 0, 1);
        auto num_pkts = pkts.size();
        for (uint16_t num_lost = 0; num_lost < num_pkts; num_lost++) {
          auto frame = get_Frame(pkts.at(0).frame_num);
          auto missing = get_missing(num_pkts, num_lost);
          for (uint16_t i = 0; i < num_pkts; i++) {
            if (not missing.at(i)) {
              frame.add_pkt(pkts.at(i));
              EXPECT_EQ(frame.get_first_sqn(), pkts.front().seq_num);
              EXPECT_EQ(frame.get_last_sqn(), pkts.at(i).seq_num);
            }
          }
          uint16_t num_use_pkts = 1;
          uint16_t highest_data = 0;
          for (uint16_t pos = 0; pos < num_pkts; pos++) {
            if (!missing.at(pos)) { num_use_pkts = pos + 1; }
            if (!pkts.at(pos).is_parity) { highest_data = pos; }
          }
          for (uint16_t i = 0; i < num_use_pkts; i++) {
            const auto test_frame_pkts = frame.get_frame_pkts();
            auto test_missing = frame.get_frame_packet_losses();
            for (uint16_t j = 0; j < num_use_pkts; j++) {
              if (!missing.at(j)) {
                EXPECT_EQ(pkts.at(j), test_frame_pkts.at(j));
                EXPECT_FALSE(test_missing.at(j));
              }
              else {
                EXPECT_EQ(test_frame_pkts.at(j).payload, "pad");
                EXPECT_TRUE(test_missing.at(j));
              }
            }
          }
          EXPECT_FALSE(frame.is_decoded());
          frame.update_size(size);
          bool any_data_lost = false;
          for (uint16_t l = 0; l <= highest_data; l++) {
            if (missing.at(l)) { any_data_lost = true; }
          }
          EXPECT_EQ(frame.is_decoded(), (num_use_pkts > highest_data) and
              !any_data_lost);
        }
      }
    }
  }
}

int main(int argc, char * argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
