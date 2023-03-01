#include "gtest/gtest.h"
#include <random>
#include <chrono>

#include "helpers.hh"
#include "wrap_helpers.hh"

using namespace std;

auto MTU = Packetization::MTU;
const std::vector<size_t> size_increment = {1, 1, 1, 1, 1, MTU - 2};

TEST(test_wrap_helpers, test_order_is_switched_no_wrap)
{
  for (uint16_t first_frame_num = 0; first_frame_num < 20; first_frame_num++) {
    for (uint32_t second_frame_num_long = first_frame_num; second_frame_num_long < 65536 - 100;
        second_frame_num_long += (65536 / 20)) {
      uint16_t second_frame_num = uint16_t(second_frame_num_long);
      EXPECT_FALSE(order_is_switched(second_frame_num, first_frame_num));
      if (first_frame_num != second_frame_num) {
        EXPECT_TRUE(order_is_switched(first_frame_num, second_frame_num));
      }
    }
  }
}

TEST(test_wrap_helpers, test_order_is_switched_wrap)
{
  for (uint16_t first_frame_num = 0; first_frame_num <= 100; first_frame_num++) {
    for (uint16_t second_frame_num = (65535 - 100 + 1); second_frame_num > 10;
        second_frame_num++) {
      EXPECT_FALSE(order_is_switched(first_frame_num, second_frame_num));
      EXPECT_FALSE(order_is_switched(second_frame_num, first_frame_num));
    }
  }
}

TEST(test_wrap_helpers, test_frames_are_out_of_order_passed_few)
{
  FrameGenerator frameGenerator = get_FrameGenerator();
  for (size_t num_frames = 0; num_frames <= 2; num_frames++) {
    deque<Frame> frames;
    for (size_t i = 0; i < num_frames; i++) {
      frames.emplace_back(get_Frame(i));
    }
    EXPECT_FALSE(frames_are_out_of_order(frames));
  }
}

TEST(test_wrap_helpers, test_frames_are_out_of_order_passed)
{
  FrameGenerator frameGenerator = get_FrameGenerator();
  srand(0);
  uint16_t frame_num = 0;
  for (uint16_t trials = 0; trials < 100; trials++) {
    for (size_t num_frames = 3; num_frames <= 30; num_frames++) {
      deque<Frame> frames;
      for (size_t i = 0; i < num_frames; i++) {
        uint16_t remaining = 10;
        while ((rand() % 2) and remaining) {
          get_Frame(frame_num++);
          remaining--;
        }
        frames.emplace_back(get_Frame(frame_num++));
      }
      EXPECT_FALSE(frames_are_out_of_order(frames));
    }
  }
}

TEST(test_wrap_helpers, test_frames_are_out_of_order_failed)
{
  FrameGenerator frameGenerator = get_FrameGenerator();
  for (uint16_t num_frames = 2; num_frames < 20; num_frames++) {
    for (uint16_t first_swap = 0; first_swap < num_frames; first_swap++) {
      for (uint16_t second_swap = first_swap+1; second_swap < num_frames;
          second_swap++) {
        deque<Frame> frames;
        for (size_t i = 0; i < num_frames; i++) {
          frames.emplace_back(get_Frame(i));
        }
        auto tmp1 = frames.at(first_swap);
        auto tmp2 = frames.at(second_swap);
        frames.at(first_swap) = tmp2;
        frames.at(second_swap) = tmp1;
        EXPECT_TRUE(frames_are_out_of_order(frames));
      }
    }
  }
}

int main(int argc, char * argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
