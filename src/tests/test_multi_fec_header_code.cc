#include "gtest/gtest.h"
#include <algorithm>
#include <random>

#include "multi_fec_header_code.hh"

using namespace std;

TEST(test_multi_fec_header_code, test_encode_sizes_of_frames)
{
  for (uint16_t delay = 1; delay < 5; delay++) {
    auto num_frames = num_frames_for_delay(delay);
    uint16_t n_cols = num_frames;
    uint16_t n_rows = 256;
    while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
    CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, 8};
    BlockCode blockCode(codingMatrixInfo, delay, pair { 1, 0 }, 8, false);
    MultiFECHeaderCode headerCode(&blockCode, delay, n_rows / num_frames);
    for (uint16_t pkt_pos_in_frame = 0; pkt_pos_in_frame < 5; pkt_pos_in_frame++) {
      uint64_t size = headerCode.encode_sizes_of_frames(delay,
          pkt_pos_in_frame);
      if (pkt_pos_in_frame == 0) { EXPECT_EQ(size, delay); }
      else { EXPECT_NE(size, delay); }
    }
  }
}

TEST(test_multi_fec_header_code, test_decode_sizes_of_frames_single_frame)
{
  for (uint16_t num_pkts = 1; num_pkts < 5; num_pkts++) {
    for (uint16_t start_pos = 0; start_pos < num_pkts - 1; start_pos++) {
      uint16_t delay = 0;
      auto num_frames = num_frames_for_delay(delay);
      uint16_t n_cols = num_frames;
      uint16_t n_rows = 255;
      while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
      CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, 8};
      BlockCode encodeBlockCode(codingMatrixInfo, delay, pair { 1, 0 }, 8, false);
      MultiFECHeaderCode encodeHeaderCode(&encodeBlockCode, delay, n_rows / num_frames);
      BlockCode decodeBlockCode(codingMatrixInfo, delay, pair { 1, 0 }, 8, false);
      MultiFECHeaderCode decodeHeaderCode(&decodeBlockCode, delay, n_rows / num_frames);
      for (uint16_t frame_num = 0; frame_num < 5; frame_num++) {
        uint64_t size = uint64_t(frame_num + 4);
        for (uint16_t pos = 0; pos <= start_pos + 1; pos++) {
          auto val = encodeHeaderCode.encode_sizes_of_frames(size, pos);
          if (pos == start_pos + 1) {
            EXPECT_NE(val, size);
            auto test_val = decodeHeaderCode.decode_sizes_of_frames(frame_num,
              pos, val);
            EXPECT_EQ(1, test_val.size());
            EXPECT_EQ(frame_num, test_val.at(0).first);
            EXPECT_EQ(size, test_val.at(0).second);
          }
        }
      }
    }
  }
}

TEST(test_multi_fec_header_code, test_decode_sizes_of_frames_single_lost_frame)
{
  for (uint16_t delay = 1; delay < 4; delay++) {
    for (uint16_t lost_frame = 0; lost_frame < 4; lost_frame++) {
      auto num_frames = num_frames_for_delay(delay);
      uint16_t n_cols = num_frames;
      uint16_t n_rows = 256;
      while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
      CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, 8};

      BlockCode encodeBlockCode(codingMatrixInfo, delay, pair { 1, 0 }, 8,
          false);
      MultiFECHeaderCode encodeHeaderCode(&encodeBlockCode, delay, n_rows / num_frames);
      BlockCode decodeBlockCode(codingMatrixInfo, delay, pair { 1, 0 }, 8,
          false);
      MultiFECHeaderCode decodeHeaderCode(&decodeBlockCode, delay, n_rows / num_frames);
      for (uint16_t frame_num = 0; frame_num < lost_frame + 2; frame_num++) {
        for (uint16_t start_pos = 0; start_pos < 2; start_pos++) {
          uint64_t size = uint64_t(frame_num + 4);
          if (frame_num == lost_frame) {
            encodeHeaderCode.encode_sizes_of_frames(size, start_pos);
          }
          else {
            auto val = encodeHeaderCode.encode_sizes_of_frames(size, start_pos);
            auto test_val = decodeHeaderCode.decode_sizes_of_frames(frame_num,
                start_pos, val);
            if (start_pos  == 0) {
              EXPECT_EQ(1, test_val.size());
              EXPECT_EQ(frame_num, test_val.at(0).first);
              EXPECT_EQ(size, test_val.at(0).second);
            }
            else if (frame_num == (lost_frame + 1)) {
              EXPECT_EQ(1, test_val.size());
              EXPECT_EQ(frame_num - 1, test_val.at(0).first);
              EXPECT_EQ(size - 1, test_val.at(0).second);
            }
            else { EXPECT_EQ(0, test_val.size()); }
          }
        }
      }
    }
  }
}

TEST(test_multi_fec_header_code, test_decode_sizes_of_frames_intersperse)
{
  for (uint16_t delay = 4; delay < 8; delay++) {
    for (uint16_t start = 0; start < 8; start++) {
      for (uint16_t pkts_per_frame = 1; pkts_per_frame < 4;
          pkts_per_frame++) {
        auto num_frames = num_frames_for_delay(delay);
        uint16_t n_cols = num_frames;
        uint16_t n_rows = 256;
        while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
        CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, 8};

        BlockCode encodeBlockCode(codingMatrixInfo, delay, pair { 1, 0 }, 8, false);
        MultiFECHeaderCode encodeHeaderCode(&encodeBlockCode, delay, n_rows / num_frames);
        BlockCode decodeBlockCode(codingMatrixInfo, delay, pair { 1, 0 }, 8, false);
        MultiFECHeaderCode decodeHeaderCode(&decodeBlockCode, delay, n_rows / num_frames);
        uint16_t num_missing = 2;
        for (uint16_t frame_num = 0; frame_num < start + 6;
            frame_num++) {
          uint64_t size = uint64_t(20 - frame_num);
          for (uint16_t frame_pos = 0; frame_pos < pkts_per_frame + (frame_num
              % 1); frame_pos++) {
            if (frame_num == start or frame_num == start + 2) {
              encodeHeaderCode.encode_sizes_of_frames(size, frame_pos);
            }
            else {
              auto val = encodeHeaderCode.encode_sizes_of_frames(size, frame_pos);
              const auto test_val = decodeHeaderCode.decode_sizes_of_frames(
                    frame_num, frame_pos, val);
              if (frame_pos  == 0) {
                EXPECT_EQ(1, test_val.size());
                EXPECT_EQ(frame_num, test_val.at(0).first);
                EXPECT_EQ(size, test_val.at(0).second);
              }
              else if ((start + 1) % 2 or pkts_per_frame > 1) {
                if ((frame_num == (start + 1) or frame_num == (start + 3))
                    and frame_pos == 1) {
                  EXPECT_EQ(1, test_val.size());
                  EXPECT_EQ(frame_num - 1, test_val.at(0).first);
                  EXPECT_EQ(uint64_t(20 - frame_num + 1),
                      test_val.at(0).second);
                }
              }
              else if (frame_num >= (start + 3) and num_missing != 0) {
                num_missing--;
                if (num_missing == 0) {
                  EXPECT_EQ(2, test_val.size());
                  for (uint16_t j = 0; j < 2; j++) {
                    auto frame = test_val.at(j).first;
                    EXPECT_TRUE(frame == start or frame == start + 2);
                    EXPECT_NE(test_val.at(1-j).first, frame);
                    EXPECT_EQ(uint64_t(20 - frame), test_val.at(j).second);
                  }
                }
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
