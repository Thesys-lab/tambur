#include "gtest/gtest.h"
#include <algorithm>
#include <random>

#include "streaming_code_packetization.hh"

using namespace std;

TEST(test_streaming_code_packetization, test_get_num_parity_stripes)
{
  map<int, pair<int, int>> redundancy_ratio{{0, {0, 1}}, {3, {1, 4}}, {6, {1, 2}}};
  for (uint16_t parity_delay = 0; parity_delay < 4; parity_delay++) {
    for (uint16_t w = 8; w < 32; w += 8)
    {
      for (uint16_t stripe_size = 8; stripe_size < 1500; stripe_size += 8)
      {
        for (uint16_t state = 0; state < 7; state += 3) {
          StreamingCodePacketization packetization(w, stripe_size,
              redundancy_ratio, Packetization::MTU, parity_delay);
          deque<uint16_t> sizes;
          deque<uint16_t> test_sizes;
          for (uint16_t frame_num = 0; frame_num < parity_delay + 10; frame_num++)
          {
            uint16_t num_data_stripes = 1 + (rand() % 10);
            auto num_parity_stripes = packetization.get_num_parity_stripes(num_data_stripes, int(state));
            test_sizes.emplace_back(num_parity_stripes);
            if (state == 0)
            {
              sizes.emplace_back(0);
            }
            else
            {
              if (parity_delay == 0) { EXPECT_GE(num_parity_stripes, 1); }
              float mult = state == 3 ? .25 : .5;
              uint16_t num_parity = uint16_t(num_data_stripes * mult);
              uint16_t mod_val = state == 3 ? 4 : 2;
              if (num_data_stripes % mod_val)
              {
                num_parity++;
              }
              sizes.emplace_back(num_parity);
            }
          }
          for (uint16_t j = 0; j < parity_delay; j++) {
            sizes.emplace_front(0);
            auto size = packetization.get_num_parity_stripes(rand() % 10, int(state));
            test_sizes.emplace_back(size);
          }
          EXPECT_EQ(test_sizes.size(), sizes.size());
          while (!test_sizes.empty()) {
            auto test = test_sizes.front();
            auto val = sizes.front();
            EXPECT_EQ(test, val);
            test_sizes.pop_front();
            sizes.pop_front();
          }
        }
      }
    }
  }
}

TEST(test_streaming_code_packetization, test_packetization_given_stripes)
{
  map<int, pair<int, int>> redundancy_ratio{{0, {0, 1}}, {1, {1, 2}}};
  for (uint16_t w = 8; w < 32; w += 8)
  {
    for (uint16_t stripe_size = 8; stripe_size < 1500; stripe_size += 8)
    {
      StreamingCodePacketization packetization(w, stripe_size, redundancy_ratio);
      for (uint16_t num_data_stripes = 1; num_data_stripes < 10; num_data_stripes++)
      {
        for (uint16_t num_parity_stripes = 0; num_parity_stripes <= num_data_stripes; num_parity_stripes++)
        {
          auto pkts = packetization.get_pkts_given_stripes(num_parity_stripes, num_data_stripes);
          uint16_t divisor = 1;
          for (uint16_t check_val = 2; check_val <= num_data_stripes; check_val++)
          {
            if ((((num_parity_stripes % check_val) == 0) or num_parity_stripes == 0) and
                ((num_data_stripes % check_val) == 0) and
                (check_val * stripe_size <= Packetization::MTU))
            {
              divisor = check_val;
            }
          }
          EXPECT_EQ(pkts.size(), num_data_stripes / divisor + num_parity_stripes / divisor);
          auto stripes_per_packet = divisor;
          EXPECT_EQ(num_data_stripes % stripes_per_packet, 0);
          EXPECT_EQ(num_parity_stripes % stripes_per_packet, 0);
          EXPECT_LE(stripes_per_packet * stripe_size, Packetization::MTU);
          for (uint16_t pos = 0; pos < pkts.size(); pos++)
          {
            EXPECT_EQ(pkts.at(pos).first, stripes_per_packet * stripe_size);
            EXPECT_EQ(pkts.at(pos).second, pos >= (num_data_stripes / stripes_per_packet));
          }
        }
      }
    }
  }
}

TEST(test_streaming_code_packetization, test_get_packetization)
{
  map<int, pair<int, int>> redundancy_ratio{{0, {0, 1}}, {1, {1, 2}}};
  for (uint16_t w = 8; w < 32; w += 8)
  {
    for (uint16_t stripe_size = 8; stripe_size < 100; stripe_size += 8)
    {
      StreamingCodePacketization packetization(w, stripe_size, redundancy_ratio);
      for (int state = 0; state < 2; state += 1)
      {
        for (uint16_t size = 0; size < 5 * Packetization::MTU; size++)
        {
          auto pkts = packetization.get_packetization(size, state);
          if (size > 0)
          {
            EXPECT_TRUE(pkts.size() > 0);
            int total = 0;
            for (auto pkt : pkts)
            {
              total += pkt.first * !pkt.second;
            }
            auto adjusted_size = size;
            if (adjusted_size % stripe_size)
            {
              adjusted_size += stripe_size;
              adjusted_size = (adjusted_size / stripe_size) * stripe_size;
            }
            EXPECT_EQ(total, adjusted_size);
          }
          else
          {
            for (auto pkt : pkts)
            {
              assert(!pkt.second);
            }
            continue;
          }
          uint16_t max_size = pkts.at(0).first;
          uint16_t total_parity = 0;
          uint16_t total_data = 0;
          uint16_t num_data = 0;
          uint16_t num_parity = 0;
          bool seen_parity = false;
          for (size_t index = 0; index < pkts.size(); index++)
          {
            const auto val = pkts.at(index);
            EXPECT_TRUE(val.first <= Packetization::MTU);
            EXPECT_EQ(val.first, max_size);
            if (!val.second)
            {
              total_data += val.first;
              num_data++;
            }
            else
            {
              EXPECT_GE(state, 1);
              total_parity += val.first;
              num_parity++;
              seen_parity = true;
            }
          }

          if (size > 0)
          {
            EXPECT_EQ(seen_parity, state > 0);
          }
          auto red = redundancy_ratio.at(state);
          EXPECT_EQ(total_parity / stripe_size, int(ceil(((total_data / stripe_size) * red.first) / float(red.second))));
        }
      }
    }
  }
}

int main(int argc, char *argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
