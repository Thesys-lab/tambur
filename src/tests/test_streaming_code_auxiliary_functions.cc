#include "gtest/gtest.h"
#include <chrono>
#include <iostream>
#include <cmath>
#include <random>
#include <thread>
#include "streaming_code.hh"
#include "helpers.hh"

using namespace std;

TEST(test_streaming_code_auxiliary_functions,
    test_final_data_stripe_of_frame_diff_size)
{
  for (uint16_t frame_size = 0; frame_size < 30; frame_size++) {
    for (uint16_t stripe_size = 1; stripe_size < 6; stripe_size++) {
      uint16_t num_data_stripes = frame_size / stripe_size;
      if (num_data_stripes * stripe_size < frame_size) {
        num_data_stripes++;
      }
      for (uint16_t rel_stripe = 0; rel_stripe < num_data_stripes; rel_stripe++) {
        bool offset = (frame_size % stripe_size) > 0;
        bool is_final_dif = offset and rel_stripe == num_data_stripes - 1;
        EXPECT_EQ(is_final_dif, final_data_stripe_of_frame_diff_size(
              frame_size, stripe_size, rel_stripe, num_data_stripes));
      }
    }
  }
}

TEST(test_streaming_code_auxiliary_functions,
    test_final_pos_within_stripe)
{
  for (uint16_t frame_size = 1; frame_size < 30; frame_size++) {
    for (uint16_t stripe_size = 1; stripe_size < 6; stripe_size++) {
      uint16_t num_data_stripes = frame_size / stripe_size;
      if (num_data_stripes * stripe_size < frame_size) {
        num_data_stripes++;
      }
      for (uint16_t rel_stripe = 0; rel_stripe < num_data_stripes; rel_stripe++) {
        bool offset = (frame_size % stripe_size) > 0;
        bool is_final_dif = offset and rel_stripe == num_data_stripes - 1;
        uint16_t num = (is_final_dif ? frame_size % stripe_size : stripe_size);
        EXPECT_EQ(num - 1, final_pos_within_stripe(
              frame_size, stripe_size, rel_stripe));
      }
    }
  }
}

TEST(test_streaming_code_auxiliary_functions,
    test_compute_num_data_stripes)
{
  for (uint16_t frame_size = 1; frame_size < 30; frame_size++) {
    for (uint16_t stripe_size = 1; stripe_size < 6; stripe_size++) {
      uint16_t num_data_stripes = 0;
      while (num_data_stripes * stripe_size < frame_size) {
        num_data_stripes++;
      }
      EXPECT_EQ(num_data_stripes, compute_num_data_stripes(frame_size, stripe_size));
    }
  }
}

TEST(test_streaming_code_auxiliary_functions,
    test_first_pos_of_frame_in_block_code_erased)
{
  for (uint tau = 0; tau < 4; tau++) {
    uint16_t num_frames = num_frames_for_delay(tau);
    for (uint16_t frame = 0; frame < 3* num_frames + 2; frame++) {
      for (uint16_t is_parity = 0; is_parity < 2; is_parity++) {
        for (uint16_t max_data_stripes = 1; max_data_stripes < 5; max_data_stripes++) {
          for (uint16_t max_fec_stripes = 1; max_fec_stripes < 5; max_fec_stripes++) {
            auto first_pos = first_pos_of_frame_in_block_code_erased(frame,
                num_frames, max_data_stripes, max_fec_stripes, is_parity);
            uint16_t max_stripes = is_parity? max_fec_stripes :
                max_data_stripes;
            uint16_t start = is_parity? max_data_stripes * num_frames : 0;
            uint16_t rel_frame = frame % num_frames;
            uint16_t actual_first_pos = start + rel_frame * max_stripes;
            EXPECT_EQ(first_pos, actual_first_pos);
          }
        }
      }
    }
  }
}

TEST(test_streaming_code_auxiliary_functions,
    test_expired_frame)
{
  for (uint16_t num_frames = 0; num_frames < 8; num_frames++) {
    for (uint16_t timeslot = 0; timeslot < 10 * num_frames; timeslot++) {
      for (uint16_t frame_num = 0; frame_num <= timeslot; frame_num++) {
        bool test_is_expired = expired_frame(frame_num, timeslot, num_frames);
        bool is_expired = timeslot >= num_frames;
        if (is_expired) {
          for (uint16_t t = timeslot - num_frames + 1; t != timeslot + 1; t++) {
            if (t == frame_num) {
              is_expired = false;
            }
          }
        }
        EXPECT_EQ(is_expired, test_is_expired);
      }
    }
  }
}

TEST(test_streaming_code_auxiliary_functions,
    test_cols_of_frame_for_size)
{
  for (uint tau = 0; tau < 4; tau++) {
    uint16_t num_frames = num_frames_for_delay(tau);
    for (uint16_t frame = 0; frame < 3* num_frames + 2; frame++) {
      for (uint16_t stripe_size = 1; stripe_size < 4; stripe_size++) {
        for (uint16_t max_data_stripes = 1; max_data_stripes < 5; max_data_stripes++) {
          for (uint16_t max_fec_stripes = 1; max_fec_stripes < 5; max_fec_stripes++) {
            for (uint16_t size = 1; size <= max_data_stripes * stripe_size;
                size++) {
              pair<uint16_t, uint16_t> cols = cols_of_frame_for_size(frame,
                  size, num_frames, stripe_size, max_data_stripes,
                  max_fec_stripes);
              uint16_t first = (frame % num_frames) * max_data_stripes;
              EXPECT_EQ(cols.first, first);
              uint16_t num_stripes = size / stripe_size;
              if (num_stripes * stripe_size < size) { num_stripes++; }
              EXPECT_EQ(cols.second, first + num_stripes - 1);
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code_auxiliary_functions,
    test_get_frame_empty)
{
  vector<FECDatagram> data_pkts;
  string empty;
  for (uint16_t frame_size = 0; frame_size < 10; frame_size++) {
    EXPECT_EQ(empty, get_frame(data_pkts, frame_size));
  }
}

TEST(test_streaming_code_auxiliary_functions,
    test_get_frame_nonempty)
{
  for (uint16_t frame_size = 1; frame_size < 61; frame_size++) {
    for (uint16_t stripe_size = 1; stripe_size < 6; stripe_size++) {
      uint16_t num_stripes = compute_num_data_stripes(frame_size, stripe_size);
      uint16_t div = 5;
      while (num_stripes % div and div > 1) { div--; }
      uint16_t num_pkts = num_stripes / div;
      string frame;
      vector<FECDatagram> data_pkts;
      for (uint16_t pkt  = 0; pkt < num_pkts; pkt++) {
        string payload(stripe_size * div, char(pkt));
        auto nxt_pkt = FECDatagram { 0, 0, 0, 0, 0, 0, payload };
        data_pkts.push_back(nxt_pkt);
        if (pkt < num_pkts -1) { frame.append(payload); }
        else {
          uint16_t pos = 0;
          while (frame.size() < frame_size) {
            frame.push_back(payload.at(pos++));
          }
        }
      }
      EXPECT_EQ(frame, get_frame(data_pkts, frame_size));
    }
  }
}

TEST(test_streaming_code_auxiliary_functions,
    test_get_stripe)
{
  for (uint16_t frame_size = 1; frame_size < 61; frame_size++) {
    for (uint16_t stripe_size = 1; stripe_size < 6; stripe_size++) {
      uint16_t num_stripes = compute_num_data_stripes(frame_size, stripe_size);
      uint16_t div = 5;
      while (num_stripes % div and div > 1) { div--; }
      uint16_t num_pkts = num_stripes / div;
      string frame;
      for (uint16_t pkt  = 0; pkt < num_pkts; pkt++) {
        string payload(stripe_size * div, char(pkt));
        if (pkt < num_pkts -1) { frame.append(payload); }
        else {
          uint16_t pos = 0;
          while (frame.size() < frame_size) {
            frame.push_back(payload.at(pos++));
          }
        }
      }
      uint16_t pos = 0;
      for (uint16_t stripe = 0; stripe < num_stripes; stripe++) {
        uint16_t relative_stripe_size = stripe_size;
        if (stripe == num_stripes - 1 and frame_size % stripe_size) {
          relative_stripe_size = frame_size % stripe_size;
        }
        auto nxt_stripe = get_stripe(frame, stripe, relative_stripe_size, stripe_size);
        while (pos < frame_size and pos < (stripe + 1) * stripe_size) {
          EXPECT_EQ(nxt_stripe.at(pos % stripe_size), frame.at(pos));
          pos += 1;
        }
      }
    }
  }
}

TEST(test_streaming_code_auxiliary_functions,
    test_get_all_payloads_and_set_payloads_and_parity_payloads)
{
  for (uint16_t delay = 0; delay < 3; delay++) {
    for (uint16_t w = 32; w < 33; w += 16) {
      for (uint16_t size_factor = 1; size_factor < 3; size_factor++) {
        uint16_t max_frame_size_possible = 1000;
        uint16_t packet_size = 8 * size_factor;
        uint16_t stripe_size = packet_size * w;
        uint16_t max_data_frame_pkts = 100;
        uint16_t max_fec_frame_pkts = 50;
        uint16_t max_data_stripes_per_frame = 128;
        uint16_t max_fec_stripes_per_frame = 32;
        assert (max_data_stripes_per_frame * stripe_size >= max_frame_size_possible);
        assert (max_fec_stripes_per_frame * stripe_size >= max_frame_size_possible / 2);
        assert (pow(2, uint64_t(w)) >= uint64_t(num_frames_for_delay(delay)) * uint64_t(num_frames_for_delay(delay))
            * max_data_stripes_per_frame * max_fec_stripes_per_frame);
        CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(delay) *
            max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(delay) * max_data_stripes_per_frame), w};
        for (uint16_t start_size = 100; start_size < max_frame_size_possible / 2; start_size += 100) {
          BlockCode blockCodeFEC(codingMatrixInfoFEC, delay,
              pair<uint16_t, uint16_t>{1, 1 }, packet_size, false);
          for (uint16_t frame_num = 0; frame_num < 10*(delay+1); frame_num++) {
            blockCodeFEC.update_timeslot(frame_num);
            uint16_t size = start_size + frame_num;
            uint16_t num_data = compute_num_data_stripes(size, stripe_size);
            auto first_stripe = frame_num % num_frames_for_delay(delay);
            for (uint16_t rel_stripe = 0; rel_stripe < num_data; rel_stripe++) {
              vector<char> vals(final_pos_within_stripe(size, stripe_size, rel_stripe) + 1, char(rel_stripe));
              blockCodeFEC.place_payload(first_stripe + rel_stripe, false, vals);
            }
            blockCodeFEC.encode();
            uint16_t total = size / 2;
            if (total % stripe_size) { total += stripe_size; }
            total -= total % stripe_size;
            string all_vals;
            auto first = first_pos_of_frame_in_block_code(frame_num, num_frames_for_delay(delay), max_data_stripes_per_frame, max_fec_stripes_per_frame, true);
            for (uint16_t st = 0; st < total / stripe_size; st++) {
              all_vals.append(blockCodeFEC.get_row(first + st, true));
            }
            string all_payloads =
            get_all_payloads(codingMatrixInfoFEC, total, frame_num, stripe_size,
                blockCodeFEC, delay);
            EXPECT_EQ(all_payloads, all_vals);
            uint16_t num_stripes = total / stripe_size;
            uint16_t div = 5;
            while (num_stripes % div and div > 1) { div--; }
            vector<string> test_payloads(num_stripes / div, "");
            test_payloads = set_payloads(codingMatrixInfoFEC,
                test_payloads, total, stripe_size * div, frame_num, stripe_size,
                blockCodeFEC, delay);
            vector<string> payloads;
            uint16_t rel_pos = 0;
            for (uint16_t j = 0; j < num_stripes / div; j++) {
              string s;
              for (uint16_t l = 0; l < div * stripe_size; l++) {
                s.push_back(all_payloads.at(rel_pos++));
              }
              payloads.push_back(s);
            }
            vector<uint16_t> parity_pkt_sizes(payloads.size(), payloads.back().size());
            EXPECT_EQ(payloads, test_payloads);
            auto test_payloads_end_to_end = parity_payloads(codingMatrixInfoFEC, frame_num, parity_pkt_sizes, stripe_size, max_fec_stripes_per_frame, blockCodeFEC, delay);
            EXPECT_EQ(payloads, test_payloads_end_to_end);
          }
        }
      }
    }
  }
}

TEST(test_streaming_code_auxiliary_functions,
    test_reserve_parity_payloads)
{
  for (uint16_t packet_size = 1; packet_size < 10; packet_size++) {
    for (uint16_t num_pkts = 1; num_pkts < 5; num_pkts++) {
      vector<uint16_t> parity_pkt_sizes(num_pkts, packet_size);
      auto payloads = reserve_parity_payloads(parity_pkt_sizes, packet_size);
      EXPECT_EQ(payloads, vector<string>(num_pkts,""));
      for (uint16_t pos = 0; pos < num_pkts; pos++) {
        parity_pkt_sizes.at(pos) = 20;
        try {
          reserve_parity_payloads(parity_pkt_sizes, packet_size);
          EXPECT_TRUE(false);
        }
        catch (...) {
          parity_pkt_sizes.at(pos) = packet_size;
        }
      }
    }
  }
}

TEST(test_streaming_code_auxiliary_functions,
    test_place_frame)
{
  for (uint16_t delay = 0; delay < 3; delay++) {
    for (uint16_t w = 32; w < 33; w += 16) {
      for (uint16_t size_factor = 1; size_factor < 3; size_factor++) {
        uint16_t max_frame_size_possible = 1000;
        uint16_t packet_size = 8 * size_factor;
        uint16_t stripe_size = packet_size * w;
        uint16_t max_data_frame_pkts = 100;
        uint16_t max_fec_frame_pkts = 50;
        uint16_t max_data_stripes_per_frame = 128;
        uint16_t max_fec_stripes_per_frame = 32;
        assert (max_data_stripes_per_frame * stripe_size >= max_frame_size_possible);
        assert (max_fec_stripes_per_frame * stripe_size >= max_frame_size_possible / 2);
        assert (pow(2, uint64_t(w)) >= uint64_t(num_frames_for_delay(delay)) * uint64_t(num_frames_for_delay(delay))
            * max_data_stripes_per_frame * max_fec_stripes_per_frame);
        CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(delay) *
            max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(delay) * max_data_stripes_per_frame), w};
        for (uint16_t start_size = 100; start_size < max_frame_size_possible / 2; start_size += 100) {
          BlockCode blockCodeFEC(codingMatrixInfoFEC, delay,
              pair<uint16_t, uint16_t>{1, 1 }, packet_size, false);
          for (uint16_t frame_num = 0; frame_num < 10*(delay+1); frame_num++) {
            blockCodeFEC.update_timeslot(frame_num);
            uint16_t size = start_size + frame_num;
            string frame;
            frame.reserve(size);
            for (uint16_t j = 0; j < size; j++) {
              frame.push_back(char(j));
            }
            place_frame(frame, stripe_size, frame_num, max_data_stripes_per_frame,
                max_fec_stripes_per_frame, num_frames_for_delay(delay), blockCodeFEC, size);
            pair<uint16_t, uint16_t> cols = cols_of_frame_for_size(frame_num, size, num_frames_for_delay(delay), stripe_size, max_data_stripes_per_frame, max_fec_stripes_per_frame);
            uint16_t pos = 0;
            while (frame.size() % stripe_size) { frame.push_back(char(0)); }
            for (uint16_t col = 0; col < cols.second - cols.first + 1; col++) {
              auto block_val = blockCodeFEC.get_row(cols.first + col, false);
              for (uint16_t i = 0; i < stripe_size; i++) {
                EXPECT_EQ(block_val.at(i), frame.at(pos++));
              }
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
