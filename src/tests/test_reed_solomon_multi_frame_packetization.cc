#include "gtest/gtest.h"
#include <algorithm>
#include <random>

#include "reed_solomon_multi_frame_packetization.hh"

using namespace std;


TEST(test_reed_solomon_multi_frame_packetization, test_count_sz)
{
  srand(1);
  map<int, pair<int, int>> redundancy_ratio {{ 0 , { 0, 1} }, { 1, { 1, 2} } };
  for (uint16_t stripe_size = 8; stripe_size < 100; stripe_size += 8) {
    for (uint16_t delay = 1; delay < 4; delay++) {
      for (uint16_t w = 8; w < 9; w += 8) {
        ReedSolomonMultiFramePacketization packetization(stripe_size, redundancy_ratio, delay + 1, w);
        for (uint16_t count_parity = 0; count_parity < 2; count_parity++) {
          for (uint16_t num_pkts = 1; num_pkts < 20; num_pkts ++) {
            for (uint16_t num_trials = 0; num_trials < 30; num_trials++) {
              uint16_t count = 0;
              vector<pair<uint16_t, bool>> pkts;
              for (uint16_t pos = 0; pos < num_pkts; pos++) {
                bool is_parity = rand() % 2;
                uint16_t size = (rand() % 10) * stripe_size;
                if (is_parity == count_parity) { count += size; }
                pkts.push_back({size, is_parity});
              }
              EXPECT_EQ(packetization.count_sz(count_parity, pkts), count);
            }
          }
        }
      }
    }
  }
}

TEST(test_reed_solomon_multi_frame_packetization, test_get_packetization_tau_0)
{
  srand(1);
  map<int, pair<int, int>> redundancy_ratio {{ 0 , { 0, 1} }, { 1, { 1, 2} } };
  for (uint16_t stripe_size = 8; stripe_size < 100; stripe_size += 8) {
    for (uint16_t size = 0; size < 5*Packetization::MTU;  size++) {
      for (uint16_t w = 8; w < 9; w += 8) {
        for (int state = 0; state < 2; state += 1) {
          ReedSolomonMultiFramePacketization packetization(stripe_size, redundancy_ratio, 1, w);
          StreamingCodePacketization streamingCodePacketization(w, stripe_size, redundancy_ratio);
          for (uint16_t frame = 0; frame < 5;  frame++) {
            auto pkts = packetization.get_packetization(size,state);
            if (size >0) {
              EXPECT_TRUE(pkts.size() > 0);
              int total = 0;
              for (auto pkt : pkts) { total += pkt.first * !pkt.second; }
              auto adjusted_size = size;
              if (adjusted_size % stripe_size) {
                adjusted_size += stripe_size;
                adjusted_size = (adjusted_size / stripe_size) * stripe_size;
              }
              EXPECT_EQ(total, adjusted_size);
            }
            else { continue; }
            uint16_t max_size = pkts.at(0).first;
            uint16_t total_parity = 0;
            uint16_t total_data = 0;
            uint16_t num_data = 0;
            uint16_t num_parity = 0;
            bool seen_parity = false;
            for (size_t index = 0; index < pkts.size(); index++) {
              const auto val = pkts.at(index);
              EXPECT_TRUE(val.first <= Packetization::MTU);
              EXPECT_EQ(val.first, max_size);
              if (!val.second) {
                total_data += val.first;
                num_data++;
              }
              else {
                EXPECT_GE(state,1);
                total_parity += val.first;
                num_parity++;
                seen_parity = true;
              }
            }
            if (size > 0) {
              EXPECT_EQ(seen_parity, state > 0);
            }
            EXPECT_EQ(total_parity / stripe_size, streamingCodePacketization.get_num_parity_stripes(total_data / stripe_size, state) );
          }
        }
      }
    }
  }
}

TEST(test_reed_solomon_multi_frame_packetization, test_get_packetization_tau_nonzero)
{
  srand(1);
  map<int, pair<int, int>> redundancy_ratio {{ 0 , { 0, 1} }, { 1, { 1, 2} } };
  for (uint16_t stripe_size = 8; stripe_size < 100; stripe_size += 8) {
    for (uint16_t size = 0; size < 5*Packetization::MTU;  size++) {
      for (uint16_t delay = 1; delay < 4; delay++) {
        for (uint16_t w = 8; w < 9; w += 8) {
          for (int state = 0; state < 2; state += 1) {
            uint16_t total_data = 0;
            ReedSolomonMultiFramePacketization packetization(stripe_size, redundancy_ratio, delay + 1, w);
            StreamingCodePacketization streamingCodePacketization(w, stripe_size, redundancy_ratio);
            for (uint16_t frame_num = 0; frame_num < 20; frame_num++) {
              if ((frame_num % (delay + 1)) == 0) {
                total_data = 0;
              }
              auto pkts = packetization.get_packetization(size,state);
              if (size >0) {
                EXPECT_TRUE(pkts.size() > 0);
                int total = 0;
                for (auto pkt : pkts) { total += pkt.first * !pkt.second; }
                auto adjusted_size = size;
                if (adjusted_size % stripe_size) {
                  adjusted_size += stripe_size;
                  adjusted_size = (adjusted_size / stripe_size) * stripe_size;
                }
                EXPECT_EQ(total, adjusted_size);
              }
              else { continue; }
              uint16_t max_size = pkts.at(0).first;
              uint16_t total_parity = 0;
              uint16_t num_data = 0;
              uint16_t num_parity = 0;
              bool seen_parity = false;
              for (size_t index = 0; index < pkts.size(); index++) {
                const auto val = pkts.at(index);
                EXPECT_TRUE(val.first <= Packetization::MTU);
                EXPECT_EQ(val.first, max_size);
                if (!val.second) {
                  total_data += val.first;
                  num_data++;
                }
                else {
                  EXPECT_GE(state,1);
                  EXPECT_TRUE(frame_num % (delay + 1)  == delay);
                  total_parity += val.first;
                  num_parity++;
                  seen_parity = true;
                }
              }
              if (state == 0 or (frame_num % (delay + 1)  != delay)) {
                EXPECT_EQ(seen_parity, 0);
              }
              if ((frame_num % (delay + 1))  == delay) {
                EXPECT_GE(total_parity / stripe_size, streamingCodePacketization.get_num_parity_stripes(total_data / stripe_size, state) );
                EXPECT_LE(total_parity / stripe_size - streamingCodePacketization.get_num_parity_stripes(total_data / stripe_size, state), delay + 1);
              }
              else { EXPECT_EQ(total_parity, 0); }
            }
          }
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
