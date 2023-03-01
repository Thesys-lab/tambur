#include "gtest/gtest.h"

#include "streaming_code_theoretical.hh"

using namespace std;

TEST(test_streaming_code_theoretical, test_add_frame_single)
{
  for (uint16_t num_missing_u = 0; num_missing_u < 10; num_missing_u++) {
    for (uint16_t num_missing_v = 0; num_missing_v < 10; num_missing_v++) {
      for (uint16_t num_received_parity = 0; num_received_parity < 20;
          num_received_parity++) {
        StreamingCodeTheoretical code(0);
        auto recover = num_received_parity >= (num_missing_u + num_missing_v);
        vector<bool> frame_u_recovered = { recover or (num_missing_u == 0) };
        vector<bool> frame_v_recovered = { recover or (num_missing_v == 0) };
        vector<bool> frame_unusable_parities = { !recover or num_received_parity == 0 };
        code.add_frame(num_missing_u, num_missing_v, num_received_parity);
        EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
        EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
        EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
      }
    }
  }
}
TEST(test_streaming_code_theoretical, test_add_frames_single)
{
  for (uint16_t num_missing_u = 0; num_missing_u < 10; num_missing_u++) {
    for (uint16_t num_missing_v = 0; num_missing_v < 10; num_missing_v++) {
      for (uint16_t num_received_parity = 0; num_received_parity < 20;
          num_received_parity++) {
        StreamingCodeTheoretical code(0);
        auto recover = num_received_parity >= (num_missing_u + num_missing_v);
        vector<bool> frame_u_recovered = { recover or (num_missing_u == 0) };
        vector<bool> frame_v_recovered = { recover or (num_missing_v == 0) };
        vector<bool> frame_unusable_parities = { !recover or num_received_parity == 0 };
        vector<uint16_t>num_missing_us = { num_missing_u };
        vector<uint16_t> num_missing_vs = { num_missing_v };
        vector<uint16_t> num_received_parities = { num_received_parity };
        code.add_frames(num_missing_us, num_missing_vs, num_received_parities, 0);
        EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
        EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
        EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
      }
    }
  }
}

TEST(test_streaming_code_theoretical, test_add_frame_add_frames_decode_MDS_fail_mult_frame_memory)
{
  uint16_t streaming_delay =  3;
  vector<uint16_t> num_missing_us = {0, 0};
  vector<uint16_t> num_missing_vs = {0, 0};
  vector<uint16_t> num_received_parities = {0, 0};
  StreamingCodeTheoretical code(streaming_delay);
  code.add_frame(0, 0, 0);
  code.add_frame(0, 0, 0);

  vector<bool> frame_u_recovered = { true, true };
  vector<bool> frame_v_recovered = { true, true };
  vector<bool> frame_unusable_parities = { true, true};

  uint16_t num_missing_u = 0;
  uint16_t num_missing_v = 34 * 3;
  uint16_t num_received_parity = 34*2;
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(false);
  frame_unusable_parities.push_back(true);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);

  num_missing_v = 34 * 1;
  num_received_parity = 34*1;
  for (uint16_t j = 0; j < 4; j++) {
    frame_u_recovered.push_back(true);
    frame_v_recovered.push_back(false);
    frame_unusable_parities.push_back(false);
    num_missing_us.push_back(num_missing_u);
    num_missing_vs.push_back(num_missing_v);
    num_received_parities.push_back(num_received_parity);
    code.add_frame(num_missing_u, num_missing_v, num_received_parity);
    EXPECT_EQ(code.frame_u_recovered(),
        frame_u_recovered);
    EXPECT_EQ(code.frame_v_recovered(),
        frame_v_recovered);
    EXPECT_EQ(code.frame_unusable_parities(),
        j < 3? frame_unusable_parities : vector<bool>(7, true));
  }
  frame_unusable_parities.at(4) = true;
  EXPECT_EQ(code.frame_u_recovered().at(1),
      frame_u_recovered.at(1));
  EXPECT_EQ(code.frame_v_recovered().at(1),
      frame_v_recovered.at(1));
  EXPECT_EQ(code.frame_unusable_parities().at(1),
      frame_unusable_parities.at(1));
  for (uint16_t timeslot = 2*streaming_delay; timeslot < 10*streaming_delay+1;
      timeslot++) {
    StreamingCodeTheoretical codeCopy(streaming_delay);
    codeCopy.add_frames(num_missing_us, num_missing_vs, num_received_parities,
        timeslot);
    EXPECT_EQ(codeCopy.frame_u_recovered(), frame_u_recovered);
    EXPECT_EQ(codeCopy.frame_v_recovered(), frame_v_recovered);
    EXPECT_EQ(codeCopy.frame_unusable_parities(), vector<bool>(7, true));
    uint16_t u = num_missing_us.back();
    uint16_t v = num_missing_vs.back();
    uint16_t p = num_received_parities.back();
    vector<uint16_t> num_missing_us_wrap = {u};
    vector<uint16_t> num_missing_vs_wrap = {v};
    vector<uint16_t> num_received_parities_wrap = {p};
    for (uint16_t j = 0; j < num_missing_us.size()-1; j++) {
      num_missing_us_wrap.push_back(num_missing_us.at(j));
      num_missing_vs_wrap.push_back(num_missing_vs.at(j));
      num_received_parities_wrap.push_back(num_received_parities.at(j));
    }
    num_missing_us = num_missing_us_wrap;
    num_missing_vs = num_missing_vs_wrap;
    num_received_parities = num_received_parities_wrap;
  }
}

TEST(test_streaming_code_theoretical, test_add_frame_add_frames_decode_MDS_mult_frame_decode)
{
  uint16_t streaming_delay =  3;
  StreamingCodeTheoretical code(streaming_delay);
  vector<uint16_t> num_missing_us;
  vector<uint16_t> num_missing_vs;
  vector<uint16_t> num_received_parities;
  for (uint16_t j = 0; j < streaming_delay; j++) {
    code.add_frame(0, 0, 0);
    num_missing_us.push_back(0);
    num_missing_vs.push_back(0);
    num_received_parities.push_back(0);
  }
  uint16_t num_missing_u = 0;
  uint16_t num_missing_v = 34;
  uint16_t num_received_parity = 34*2;
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  vector<bool> frame_u_recovered = { true, true, true, true };
  vector<bool> frame_v_recovered = { true, true, true, true };
  vector<bool> frame_unusable_parities = { true, true, true, false};
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);

  num_missing_v = 34 * 5;
  num_received_parity = 34*2;
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(false);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);

  num_missing_v = 0;
  num_received_parity = 34*2;
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  frame_v_recovered.at(4) = true;
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  for (uint16_t timeslot = 2*streaming_delay; timeslot < 10*streaming_delay+1;
      timeslot++) {
    StreamingCodeTheoretical codeCopy(streaming_delay);
    codeCopy.add_frames(num_missing_us, num_missing_vs, num_received_parities,
        timeslot);
    EXPECT_EQ(codeCopy.frame_u_recovered(), frame_u_recovered);
    EXPECT_EQ(codeCopy.frame_v_recovered(), frame_v_recovered);
    EXPECT_EQ(codeCopy.frame_unusable_parities(), frame_unusable_parities);
    uint16_t u = num_missing_us.back();
    uint16_t v = num_missing_vs.back();
    uint16_t p = num_received_parities.back();
    vector<uint16_t> num_missing_us_wrap = {u};
    vector<uint16_t> num_missing_vs_wrap = {v};
    vector<uint16_t> num_received_parities_wrap = {p};
    for (uint16_t j = 0; j < num_missing_us.size()-1; j++) {
      num_missing_us_wrap.push_back(num_missing_us.at(j));
      num_missing_vs_wrap.push_back(num_missing_vs.at(j));
      num_received_parities_wrap.push_back(num_received_parities.at(j));
    }
    num_missing_us = num_missing_us_wrap;
    num_missing_vs = num_missing_vs_wrap;
    num_received_parities = num_received_parities_wrap;
  }
}


TEST(test_streaming_code_theoretical, test_add_frame_add_frames_decode_MDS_mult_frame_non_decode)
{
  uint16_t streaming_delay =  3;
  StreamingCodeTheoretical code(streaming_delay);
  vector<uint16_t> num_missing_us;
  vector<uint16_t> num_missing_vs;
  vector<uint16_t> num_received_parities;
  for (uint16_t j = 0; j < streaming_delay; j++) {
    code.add_frame(0, 0, 0);
    num_missing_us.push_back(0);
    num_missing_vs.push_back(0);
    num_received_parities.push_back(0);
  }
  uint16_t num_missing_u = 0;
  uint16_t num_missing_v = 34;
  uint16_t num_received_parity = 34*2;
  vector<bool> frame_u_recovered = { true, true, true, true };
  vector<bool> frame_v_recovered = { true, true, true, true };
  vector<bool> frame_unusable_parities = { true, true, true, false};
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);

  num_missing_v = 34 * 5;
  num_received_parity = 34;
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(false);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);

  num_missing_v = 0;
  num_received_parity = 34;
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  frame_unusable_parities.at(6) = true;
  frame_unusable_parities.at(5) = true;
  frame_unusable_parities.at(4) = true;
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  for (uint16_t timeslot = 2*streaming_delay; timeslot < 10*streaming_delay+1;
      timeslot++) {
    StreamingCodeTheoretical codeCopy(streaming_delay);
    codeCopy.add_frames(num_missing_us, num_missing_vs, num_received_parities,
        timeslot);
    EXPECT_EQ(codeCopy.frame_u_recovered(), frame_u_recovered);
    EXPECT_EQ(codeCopy.frame_v_recovered(), frame_v_recovered);
    EXPECT_EQ(codeCopy.frame_unusable_parities(), frame_unusable_parities);
    uint16_t u = num_missing_us.back();
    uint16_t v = num_missing_vs.back();
    uint16_t p = num_received_parities.back();
    vector<uint16_t> num_missing_us_wrap = {u};
    vector<uint16_t> num_missing_vs_wrap = {v};
    vector<uint16_t> num_received_parities_wrap = {p};
    for (uint16_t j = 0; j < num_missing_us.size()-1; j++) {
      num_missing_us_wrap.push_back(num_missing_us.at(j));
      num_missing_vs_wrap.push_back(num_missing_vs.at(j));
      num_received_parities_wrap.push_back(num_received_parities.at(j));
    }
    num_missing_us = num_missing_us_wrap;
    num_missing_vs = num_missing_vs_wrap;
    num_received_parities = num_received_parities_wrap;
  }
}


TEST(test_streaming_code_theoretical, test_add_frame_add_frames_decode_stream_mult_frame_decode_no_decode_u)
{
  for (uint16_t i = 0; i < 2; i++) {
    uint16_t streaming_delay =  3;
    vector<uint16_t> num_missing_us;
    vector<uint16_t> num_missing_vs;
    vector<uint16_t> num_received_parities;
    StreamingCodeTheoretical code(streaming_delay);
    for (uint16_t j = 0; j < streaming_delay; j++) {
      code.add_frame(0, 0, 0);
      num_missing_us.push_back(0);
      num_missing_vs.push_back(0);
      num_received_parities.push_back(0);
    }
    uint16_t num_missing_u = 24*3;
    uint16_t num_missing_v = 10*3;
    uint16_t num_received_parity = 34*2;
    num_missing_us.push_back(num_missing_u);
    num_missing_vs.push_back(num_missing_v);
    num_received_parities.push_back(num_received_parity);
    vector<bool> frame_u_recovered = { true, true, true, false };
    vector<bool> frame_v_recovered = { true, true, true, false };
    vector<bool> frame_unusable_parities = { true, true, true, false};
    code.add_frame(num_missing_u, num_missing_v, num_received_parity);
    EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
    EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
    EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
    num_missing_u = 0;
    num_missing_v = 0;
    num_received_parity = 2*34;
    frame_v_recovered.back() = true;
    frame_u_recovered.push_back(true);
    frame_v_recovered.push_back(true);
    frame_unusable_parities.push_back(false);
    num_missing_us.push_back(num_missing_u);
    num_missing_vs.push_back(num_missing_v);
    num_received_parities.push_back(num_received_parity);
    code.add_frame(num_missing_u, num_missing_v, num_received_parity);
    EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
    EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
    EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);

    frame_u_recovered.push_back(true);
    frame_v_recovered.push_back(true);
    frame_unusable_parities.push_back(false);
    num_missing_us.push_back(num_missing_u);
    num_missing_vs.push_back(num_missing_v);
    num_received_parities.push_back(num_received_parity);
    code.add_frame(num_missing_u, num_missing_v, num_received_parity);
    EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
    EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
    EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);

    num_missing_u = 22*3;
    num_missing_v = 0;
    num_received_parity = 3*24 - 2*34 + 22*3 -1 + i;
    frame_u_recovered.push_back(i > 0);
    frame_v_recovered.push_back(true);
    frame_unusable_parities.push_back(i == 0);
    num_missing_us.push_back(num_missing_u);
    num_missing_vs.push_back(num_missing_v);
    num_received_parities.push_back(num_received_parity);
    code.add_frame(num_missing_u, num_missing_v, num_received_parity);
    frame_u_recovered.at(3) = i > 0;
    frame_unusable_parities.at(3) = i == 0;
    EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
    EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
    EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
    for (uint16_t timeslot = 2*streaming_delay; timeslot < 10*streaming_delay+1;
        timeslot++) {
      StreamingCodeTheoretical codeCopy(streaming_delay);
      codeCopy.add_frames(num_missing_us, num_missing_vs, num_received_parities,
          timeslot);
      EXPECT_EQ(codeCopy.frame_u_recovered(), frame_u_recovered);
      EXPECT_EQ(codeCopy.frame_v_recovered(), frame_v_recovered);
      EXPECT_EQ(codeCopy.frame_unusable_parities(), frame_unusable_parities);
      uint16_t u = num_missing_us.back();
      uint16_t v = num_missing_vs.back();
      uint16_t p = num_received_parities.back();
      vector<uint16_t> num_missing_us_wrap = {u};
      vector<uint16_t> num_missing_vs_wrap = {v};
      vector<uint16_t> num_received_parities_wrap = {p};
      for (uint16_t j = 0; j < num_missing_us.size()-1; j++) {
        num_missing_us_wrap.push_back(num_missing_us.at(j));
        num_missing_vs_wrap.push_back(num_missing_vs.at(j));
        num_received_parities_wrap.push_back(num_received_parities.at(j));
      }
      num_missing_us = num_missing_us_wrap;
      num_missing_vs = num_missing_vs_wrap;
      num_received_parities = num_received_parities_wrap;
    }
  }
}

TEST(test_streaming_code_theoretical, test_add_frame_add_frames_decode_stream_mult_frame_decode_no_decode_middle_u)
{
  uint16_t streaming_delay =  3;
  vector<uint16_t> num_missing_us;
  vector<uint16_t> num_missing_vs;
  vector<uint16_t> num_received_parities;
  StreamingCodeTheoretical code(streaming_delay);
  for (uint16_t j = 0; j < streaming_delay; j++) {
    code.add_frame(0, 0, 0);
    num_missing_us.push_back(0);
    num_missing_vs.push_back(0);
    num_received_parities.push_back(0);
  }
  vector<bool> frame_u_recovered = { true, true, true };
  vector<bool> frame_v_recovered = { true, true, true };
  vector<bool> frame_unusable_parities = { true, true, true};
  uint16_t num_missing_u = 16*5;
  uint16_t num_missing_v = (34 - 16) * 5;
  uint16_t num_received_parity = 2*34;
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  frame_u_recovered.push_back(false);
  frame_v_recovered.push_back(false);
  frame_unusable_parities.push_back(false);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  num_missing_u = 0;
  num_missing_v = 0;
  num_received_parity = 2*34;
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  frame_v_recovered.at(3) = true;
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  frame_u_recovered.at(3) = true;
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  for (uint16_t timeslot = 2*streaming_delay; timeslot < 10*streaming_delay+1;
      timeslot++) {
    StreamingCodeTheoretical codeCopy(streaming_delay);
    codeCopy.add_frames(num_missing_us, num_missing_vs, num_received_parities,
        timeslot);
    EXPECT_EQ(codeCopy.frame_u_recovered(), frame_u_recovered);
    EXPECT_EQ(codeCopy.frame_v_recovered(), frame_v_recovered);
    EXPECT_EQ(codeCopy.frame_unusable_parities(), frame_unusable_parities);
    uint16_t u = num_missing_us.back();
    uint16_t v = num_missing_vs.back();
    uint16_t p = num_received_parities.back();
    vector<uint16_t> num_missing_us_wrap = {u};
    vector<uint16_t> num_missing_vs_wrap = {v};
    vector<uint16_t> num_received_parities_wrap = {p};
    for (uint16_t j = 0; j < num_missing_us.size()-1; j++) {
      num_missing_us_wrap.push_back(num_missing_us.at(j));
      num_missing_vs_wrap.push_back(num_missing_vs.at(j));
      num_received_parities_wrap.push_back(num_received_parities.at(j));
    }
    num_missing_us = num_missing_us_wrap;
    num_missing_vs = num_missing_vs_wrap;
    num_received_parities = num_received_parities_wrap;
  }
}

TEST(test_streaming_code_theoretical, test_add_frame_add_frames_decode_stream_mult_frame_decode_decode_middle_u)
{
  uint16_t streaming_delay =  3;
  vector<uint16_t> num_missing_us;
  vector<uint16_t> num_missing_vs;
  vector<uint16_t> num_received_parities;
  StreamingCodeTheoretical code(streaming_delay);
  for (uint16_t j = 0; j < streaming_delay; j++) {
    code.add_frame(0, 0, 0);
    num_missing_us.push_back(0);
    num_missing_vs.push_back(0);
    num_received_parities.push_back(0);
  }
  vector<bool> frame_u_recovered = { true, true, true };
  vector<bool> frame_v_recovered = { true, true, true };
  vector<bool> frame_unusable_parities = { true, true, true};
  uint16_t num_missing_u = 10;
  uint16_t num_missing_v = 24;
  uint16_t num_received_parity = 2*34;
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  num_missing_u = 5 * 10;
  num_missing_v = 5 * 24;
  num_received_parity = 2*34;
  frame_u_recovered.push_back(false);
  frame_v_recovered.push_back(false);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);

  num_missing_u = 0;
  num_missing_v = 0;
  num_received_parity = 2*34;
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  frame_u_recovered.at(4) = true;
  frame_v_recovered.at(4) = true;
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  for (uint16_t timeslot = 2*streaming_delay; timeslot < 10*streaming_delay+1;
      timeslot++) {
    StreamingCodeTheoretical codeCopy(streaming_delay);
    codeCopy.add_frames(num_missing_us, num_missing_vs, num_received_parities,
        timeslot);
    EXPECT_EQ(codeCopy.frame_u_recovered(), frame_u_recovered);
    EXPECT_EQ(codeCopy.frame_v_recovered(), frame_v_recovered);
    EXPECT_EQ(codeCopy.frame_unusable_parities(), frame_unusable_parities);
    uint16_t u = num_missing_us.back();
    uint16_t v = num_missing_vs.back();
    uint16_t p = num_received_parities.back();
    vector<uint16_t> num_missing_us_wrap = {u};
    vector<uint16_t> num_missing_vs_wrap = {v};
    vector<uint16_t> num_received_parities_wrap = {p};
    for (uint16_t j = 0; j < num_missing_us.size()-1; j++) {
      num_missing_us_wrap.push_back(num_missing_us.at(j));
      num_missing_vs_wrap.push_back(num_missing_vs.at(j));
      num_received_parities_wrap.push_back(num_received_parities.at(j));
    }
    num_missing_us = num_missing_us_wrap;
    num_missing_vs = num_missing_vs_wrap;
    num_received_parities = num_received_parities_wrap;
  }
}

TEST(test_streaming_code_theoretical, test_add_frame_decode_stream_cascade_decode_first_u)
{
  uint16_t streaming_delay =  3;
  StreamingCodeTheoretical code(streaming_delay);
  for (uint16_t j = 0; j < streaming_delay; j++) { code.add_frame(0, 0, 0); }
  vector<bool> frame_u_recovered = { true, true, true };
  vector<bool> frame_v_recovered = { true, true, true };
  vector<bool> frame_unusable_parities = { true, true, true};
  uint16_t num_missing_u = 10 * 3;
  uint16_t num_missing_v = 24 * 3;
  uint16_t num_received_parity = 34;
  frame_u_recovered.push_back(false);
  frame_v_recovered.push_back(false);
  frame_unusable_parities.push_back(false);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  num_missing_u = 0;
  num_missing_v = 0;
  num_received_parity = 2*34;
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  frame_u_recovered.at(3) = true;
  frame_v_recovered.at(3) = true;
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
}

TEST(test_streaming_code_theoretical, test_add_frame_add_frames_decode_stream_mult_frame_non_decode)
{
  uint16_t streaming_delay =  3;
  vector<uint16_t> num_missing_us;
  vector<uint16_t> num_missing_vs;
  vector<uint16_t> num_received_parities;
  StreamingCodeTheoretical code(streaming_delay);
  for (uint16_t j = 0; j < streaming_delay; j++) {
    code.add_frame(0, 0, 0);
    num_missing_us.push_back(0);
    num_missing_vs.push_back(0);
    num_received_parities.push_back(0);
  }
  vector<bool> frame_u_recovered = { true, true, true };
  vector<bool> frame_v_recovered = { true, true, true };
  vector<bool> frame_unusable_parities = { true, true, true};
  uint16_t num_missing_u = 10 * 5;
  uint16_t num_missing_v = 24 * 5;
  uint16_t num_received_parity = 34;
  frame_u_recovered.push_back(false);
  frame_v_recovered.push_back(false);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  num_missing_u = 0;
  num_missing_v = 0;
  num_received_parity = 34;
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  frame_u_recovered.push_back(true);
  frame_v_recovered.push_back(true);
  frame_unusable_parities.push_back(false);
  num_missing_us.push_back(num_missing_u);
  num_missing_vs.push_back(num_missing_v);
  num_received_parities.push_back(num_received_parity);
  code.add_frame(num_missing_u, num_missing_v, num_received_parity);
  for (uint16_t j = 3; j < 7; j++) {
    frame_unusable_parities.at(j) = true;
  }
  EXPECT_EQ(code.frame_u_recovered(), frame_u_recovered);
  EXPECT_EQ(code.frame_v_recovered(), frame_v_recovered);
  EXPECT_EQ(code.frame_unusable_parities(), frame_unusable_parities);
  for (uint16_t timeslot = 2*streaming_delay; timeslot < 10*streaming_delay+1;
      timeslot++) {
    StreamingCodeTheoretical codeCopy(streaming_delay);
    codeCopy.add_frames(num_missing_us, num_missing_vs, num_received_parities,
        timeslot);
    EXPECT_EQ(codeCopy.frame_u_recovered(), frame_u_recovered);
    EXPECT_EQ(codeCopy.frame_v_recovered(), frame_v_recovered);
    EXPECT_EQ(codeCopy.frame_unusable_parities(), frame_unusable_parities);
    uint16_t u = num_missing_us.back();
    uint16_t v = num_missing_vs.back();
    uint16_t p = num_received_parities.back();
    vector<uint16_t> num_missing_us_wrap = {u};
    vector<uint16_t> num_missing_vs_wrap = {v};
    vector<uint16_t> num_received_parities_wrap = {p};
    for (uint16_t j = 0; j < num_missing_us.size()-1; j++) {
      num_missing_us_wrap.push_back(num_missing_us.at(j));
      num_missing_vs_wrap.push_back(num_missing_vs.at(j));
      num_received_parities_wrap.push_back(num_received_parities.at(j));
    }
    num_missing_us = num_missing_us_wrap;
    num_missing_vs = num_missing_vs_wrap;
    num_received_parities = num_received_parities_wrap;
  }
}

int main(int argc, char * argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
