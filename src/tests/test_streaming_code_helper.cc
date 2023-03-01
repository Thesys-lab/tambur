#include "gtest/gtest.h"

#include "streaming_code_helper.hh"
#include "helpers.hh"

using namespace std;

TEST(test_streaming_code_helper, test_get_recoverable_data_info)
{
  srand(0);
  for (uint16_t trial = 0; trial < 100; trial++) {
    for (uint16_t n_par_p_f = 1; n_par_p_f < 3; n_par_p_f++) {
      for (uint16_t n_data_p_f = 1; n_data_p_f < 6; n_data_p_f++) {
        for (uint16_t delay = 0; delay < 4; delay++) {
          for (uint16_t w = 8; w < 33; w += 8) {
            auto num_frames = num_frames_for_delay(delay);
            auto n_cols = uint16_t(n_data_p_f * num_frames);
            if (n_cols >= 256) { continue; }
            CodingMatrixInfo codingMatrixInfo { uint16_t(n_par_p_f * num_frames),
                n_cols, w };
            StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
            vector<bool> recoverable_data;
            vector<bool> erased = get_missing((uint8_t) codingMatrixInfo.n_cols,
                (uint8_t) (rand() % (n_cols)));
            vector<bool> recovered_us = get_missing((uint8_t) num_frames,
                (uint8_t) (rand() % (num_frames)));
            vector<bool> recovered_vs = get_missing((uint8_t) num_frames,
                (uint8_t) (rand() % (num_frames)));
            for (uint16_t col = 0; col < n_cols; col++) {
              auto frame = frame_of_col(col, delay, codingMatrixInfo);
              bool recover_part = helper.get_positions_of_us().at(col)?
                  recovered_us.at(frame) : recovered_vs.at(frame);
              recoverable_data.push_back(recover_part and erased.at(col));
            }
            auto test_val = helper.get_recoverable_data_info(num_frames,
                erased, recovered_us, recovered_vs);
            EXPECT_EQ(test_val.size(), recoverable_data.size());
            EXPECT_EQ(test_val.size(), n_cols);
            for (uint16_t j = 0; j < n_cols; j++) {
              EXPECT_EQ(test_val.at(j), recoverable_data.at(j));
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code_helper, test_zero_by_frame_u)
{
  for (uint16_t n_par_p_f = 1; n_par_p_f < 4; n_par_p_f++) {
    for (uint16_t n_data_p_f = 1; n_data_p_f < 4; n_data_p_f++) {
      for (uint16_t delay = 0; delay < 4; delay++) {
        for (uint16_t w = 8; w < 33; w += 8) {
          auto num_frames = num_frames_for_delay(delay);
          auto n_cols = uint16_t(n_data_p_f * num_frames);
          if (n_cols >= 256) { continue; }
          CodingMatrixInfo codingMatrixInfo { uint16_t(n_par_p_f * num_frames),
              n_cols, w };
          StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
          for (uint16_t row = 0; row < codingMatrixInfo.n_rows; row++) {
            for (uint16_t col = 0; col < codingMatrixInfo.n_cols; col++) {
              Position position { row, col };
              auto test_val = helper.zero_by_frame_u(position, codingMatrixInfo);
              if (zero_by_frame(position, codingMatrixInfo, delay)) {
                EXPECT_TRUE(test_val);
              }
              else {
                if (!helper.get_positions_of_us().at(col)) {
                  EXPECT_FALSE(test_val);
                }
                else {
                  auto frame_row = frame_of_row(row, delay, codingMatrixInfo);
                  auto frame_col = frame_of_col(col, delay, codingMatrixInfo);
                  EXPECT_EQ(!test_val, (frame_row == frame_col or ((frame_row %
                      num_frames) == ((frame_col + delay) % num_frames) )));
                }
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code_helper, test_set_vs_and_us)
{
  for (uint16_t n_par_p_f = 1; n_par_p_f < 4; n_par_p_f++) {
    for (uint16_t n_data_p_f = 2; n_data_p_f < 10; n_data_p_f++) {
      for (uint16_t delay = 0; delay < 7; delay++) {
        for (uint16_t w = 8; w < 33; w += 8) {
          for (uint16_t v = 1; v < n_data_p_f; v++) {
            for (uint16_t u = 0; u < n_data_p_f; u++) {
              auto num_frames = num_frames_for_delay(delay);
              CodingMatrixInfo codingMatrixInfo { uint16_t( n_par_p_f *
                  num_frames), uint16_t(n_data_p_f * num_frames), w };
              StreamingCodeHelper helper(codingMatrixInfo, delay, { v, u });
              vector<bool> pos_of_us;
              if (u == 0) {
                pos_of_us = vector<bool>(num_frames * n_data_p_f, false);
              }
              else {
                for (uint16_t frame = 0; frame < num_frames; frame++) {
                  uint16_t pos = 0;
                  while (pos < n_data_p_f) {
                    for (uint16_t v_val = 0; v_val < v; v_val++) {
                      if (pos < n_data_p_f and (v_val < n_data_p_f)) {
                        pos_of_us.push_back(false);
                        pos++;
                      }
                    }
                    for (uint16_t u_val = 0; u_val < u; u_val++) {
                      if (pos < n_data_p_f and (v + u_val < n_data_p_f)) {
                        pos_of_us.push_back(true);
                        pos++;
                      }
                    }
                  }
                }
              }
              EXPECT_EQ(helper.get_positions_of_us().size(), pos_of_us.size());
              for (uint16_t l = 0; l < codingMatrixInfo.n_cols; l++) {
                EXPECT_EQ(helper.get_positions_of_us().at(l), pos_of_us.at(l));
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code_helper, test_lost_data_count)
{
  srand(0);
  for (uint16_t trial = 0; trial < 1000; trial++) {
    for (uint16_t n_par_p_f = 1; n_par_p_f < 4; n_par_p_f++) {
      for (uint16_t n_data_p_f = 1; n_data_p_f < 4; n_data_p_f++) {
        for (uint16_t delay = 0; delay < 4; delay++) {
          for (uint16_t w = 8; w < 33; w += 8) {
            auto num_frames = num_frames_for_delay(delay);
            auto n_cols = uint16_t(n_data_p_f * num_frames);
            auto n_rows = uint16_t(n_par_p_f * num_frames);
            if (n_cols >= 256) { continue; }
            CodingMatrixInfo codingMatrixInfo { n_rows,
                n_cols, w };
            StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
            vector<bool> erased = get_missing((uint8_t) n_cols + n_rows,
                (uint8_t) (rand() % (n_cols + n_rows)));
            auto u_pos = helper.get_positions_of_us();
            vector<uint16_t> lost_u(num_frames, 0);
            vector<uint16_t> lost_v(num_frames, 0);
            for (uint16_t pos = 0; pos < n_cols; pos++) {
              if (erased.at(pos) and u_pos.at(pos)) {
                lost_u.at(frame_of_col(pos, delay, codingMatrixInfo)) += 1;
              }
              if (erased.at(pos) and !u_pos.at(pos)) {
                lost_v.at(frame_of_col(pos, delay, codingMatrixInfo)) += 1;
              }
            }
            auto test_lost_u = helper.lost_data_count(true, erased);
            auto test_lost_v = helper.lost_data_count(false, erased);
            EXPECT_EQ(test_lost_u.size(), num_frames);
            EXPECT_EQ(test_lost_v.size(), num_frames);
            for (uint16_t frame = 0; frame < num_frames; frame++) {
              EXPECT_EQ(test_lost_u.at(frame), lost_u.at(frame));
              EXPECT_EQ(test_lost_v.at(frame), lost_v.at(frame));
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code_helper, test_received_parities_count)
{
  for (uint16_t n_par_p_f = 1; n_par_p_f < 4; n_par_p_f++) {
    for (uint16_t n_data_p_f = 1; n_data_p_f < 4; n_data_p_f++) {
      for (uint16_t delay = 0; delay < 4; delay++) {
        for (uint16_t w = 8; w < 33; w += 8) {
          auto num_frames = num_frames_for_delay(delay);
          auto n_cols = uint16_t(n_data_p_f * num_frames);
          auto n_rows = uint16_t(n_par_p_f * num_frames);
          if (n_cols >= 256) { continue; }
          CodingMatrixInfo codingMatrixInfo { n_rows,
              n_cols, w };
          StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
          vector<bool> erased = get_missing((uint8_t) n_cols + n_rows,
              (uint8_t) (rand() % (n_cols + n_rows)));
          vector<uint16_t> parities(num_frames, n_par_p_f);
          for (uint16_t row = 0; row < n_rows; row++) {
            if (erased.at(row + n_cols)) {
              parities.at(frame_of_row(row, delay, codingMatrixInfo)) -= 1;
            }
          }
          auto test_parities = helper.received_parities_count(erased);
          EXPECT_EQ(test_parities.size(), num_frames);
          for (uint16_t frame = 0; frame < num_frames; frame++) {
            EXPECT_EQ(test_parities.at(frame), parities.at(frame));
          }
        }
      }
    }
  }
}

TEST(test_streaming_code_helper, test_get_retained_parities_zero_by_memory)
{
  for (uint16_t n_par_p_f = 1; n_par_p_f < 4; n_par_p_f++) {
    for (uint16_t n_data_p_f = 1; n_data_p_f < 4; n_data_p_f++) {
      for (uint16_t delay = 0; delay < 4; delay++) {
        for (uint16_t w = 8; w < 33; w += 8) {
          auto num_frames = num_frames_for_delay(delay);
          auto n_cols = uint16_t(n_data_p_f * num_frames);
          if (n_cols >= 256) { continue; }
          CodingMatrixInfo codingMatrixInfo { uint16_t(n_par_p_f * num_frames),
              n_cols, w };
          StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
          for (uint16_t timeslot = 0; timeslot < 4 * num_frames; timeslot++) {
            auto test_vals = helper.get_retained_parities_zero_by_memory(
                timeslot);
            vector<bool> frames_retained(num_frames, true);
            for (uint16_t shift = 1; shift <= delay; shift++) {
              frames_retained.at((timeslot + shift) % num_frames) = false;
            }
            vector<bool> vals(codingMatrixInfo.n_rows, false);
            for (uint16_t pos = timeslot; pos < timeslot + num_frames; pos++) {
              auto endpoints = rows_of_frame(pos, delay, codingMatrixInfo);
              bool val = frames_retained.at(pos % num_frames);
              for (uint16_t par = endpoints.first; par <= endpoints.second; par++) {
                vals.at(par) = val;
              }
            }
            EXPECT_EQ(test_vals.size(), vals.size());
            uint16_t total = 0;
            for (uint16_t pos = 0; pos < codingMatrixInfo.n_rows; pos++) {
              EXPECT_EQ(test_vals.at(pos), vals.at(pos));
              total += test_vals.at(pos);
            }
            EXPECT_EQ(total, (delay + 1) * n_par_p_f);
          }
        }
      }
    }
  }
}

TEST(test_streaming_code_helper, test_get_recoverable_data_burst_full_recover)
{
  for (uint16_t timeslot = 6; timeslot < 70; timeslot++) {
    for (uint16_t w = 8; w < 33; w += 8) {
      uint16_t delay = 3;
      uint16_t n_data_p_f = 8;
      uint16_t n_par_p_f = 8;
      auto num_frames = num_frames_for_delay(delay);
      auto n_cols = uint16_t(n_data_p_f * num_frames);
      auto n_rows = uint16_t(n_par_p_f * num_frames);
      CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
      StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 0 });
      auto retained_parities_by_mem = helper.
          get_retained_parities_zero_by_memory(timeslot);
      vector<bool> frame_erased_orig = { false, false, false, true, true, false, false};
      uint16_t shift = (timeslot - 6) % num_frames;
      vector<bool> frame_erased(7, false);
      for (uint16_t pos = 0; pos < 7; pos++) {
        frame_erased.at(pos) = frame_erased_orig.at((pos + shift) % num_frames);
      }
      StreamingCodeTheoretical streamingCodeTheoretical(delay);
      vector<bool> erased(n_rows + n_cols, false);
      vector<bool> parity = helper.get_retained_parities_zero_by_memory(timeslot);
      for (uint16_t t = timeslot - 3; t <= timeslot - 2; t++) {
        auto endpointsCol = cols_of_frame(t, delay, codingMatrixInfo);
        for (uint16_t i = endpointsCol.first; i <= endpointsCol.second; i++) {
          erased.at(i) = true;
        }
        auto endpointsRow = rows_of_frame(t, delay, codingMatrixInfo);
        for (uint16_t i = endpointsRow.first; i <= endpointsRow.second; i++) {
          erased.at(n_cols + i) = true;
          parity.at(i) = false;
        }
      }
      for (uint16_t t = timeslot - 6 ; t <= timeslot - 4; t++) {
        auto endpointsRow = rows_of_frame(t, delay, codingMatrixInfo);
        for (uint16_t i = endpointsRow.first; i <= endpointsRow.second; i++) {
          parity.at(i) = false;
        }
      }
      vector<bool> data(n_cols, false);
      auto endpoints = cols_of_frame(timeslot - 2, delay, codingMatrixInfo);
      for (uint16_t i = endpoints.first; i <= endpoints.second; i++) {
        data.at(i) = true;
      }
      endpoints = cols_of_frame(timeslot - 3, delay, codingMatrixInfo);
      for (uint16_t i = endpoints.first; i <= endpoints.second; i++) {
        data.at(i) = true;
      }
      auto test_val = helper.get_recoverable_data(erased, timeslot);
      EXPECT_EQ(test_val.first.size(), n_cols);
      for (uint16_t pos = 0; pos < n_cols; pos++) {
        EXPECT_EQ(test_val.first.at(pos), data.at(pos));
      }
      EXPECT_EQ(test_val.second.size(), n_rows);
      for (uint16_t pos = 0; pos < n_rows; pos++) {
        EXPECT_EQ(test_val.second.at(pos), parity.at(pos));
      }
    }
  }
}

TEST(test_streaming_code_helper, test_get_recoverable_data_burst_partial_recover)
{
  for (uint16_t timeslot = 6; timeslot < 70; timeslot++) {
    for (uint16_t w = 8; w < 9; w += 8) {
      uint16_t delay = 3;
      uint16_t n_data_p_f = 8;
      uint16_t n_par_p_f = 6;
      auto num_frames = num_frames_for_delay(delay);
      uint16_t timeslot = 2 * num_frames - 1;
      auto n_cols = uint16_t(n_data_p_f * num_frames);
      auto n_rows = uint16_t(n_par_p_f * num_frames);
      CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
      StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
      auto retained_parities_by_mem = helper.
          get_retained_parities_zero_by_memory(timeslot);
      vector<bool> frame_erased_orig = { false, false, false, true,true, false,
          false};
      uint16_t shift = (timeslot - 6) % num_frames;
      vector<bool> frame_erased(7, false);
      for (uint16_t pos = 0; pos < 7; pos++) {
        frame_erased.at(pos) = frame_erased_orig.at((pos + shift) % num_frames);
      }
      StreamingCodeTheoretical streamingCodeTheoretical(delay);
      vector<bool> erased(n_rows + n_cols, false);
      vector<bool> parity = helper.get_retained_parities_zero_by_memory(timeslot);
      for (uint16_t t = timeslot - 3; t <= timeslot - 2; t++) {
        auto endpointsCol = cols_of_frame(t, delay, codingMatrixInfo);
        for (uint16_t i = endpointsCol.first; i <= endpointsCol.second; i++) {
          erased.at(i) = true;
        }
        auto endpointsRow = rows_of_frame(t, delay, codingMatrixInfo);
        for (uint16_t i = endpointsRow.first; i <= endpointsRow.second; i++) {
          erased.at(n_cols + i) = true;
          parity.at(i) = false;
        }
      }
      vector<bool> data(n_cols, false);
      auto endpoints = cols_of_frame(timeslot - 2, delay, codingMatrixInfo);
      for (uint16_t i = endpoints.first; i <= endpoints.second; i++) {
        if (!helper.get_positions_of_us().at(i)) { data.at(i) = true; }
      }
      endpoints = cols_of_frame(timeslot - 3, delay, codingMatrixInfo);
      for (uint16_t i = endpoints.first; i <= endpoints.second; i++) {
        data.at(i) = true;
      }
      auto test_val = helper.get_recoverable_data(erased, timeslot);
      EXPECT_EQ(test_val.first.size(), n_cols);
      for (uint16_t pos = 0; pos < n_cols; pos++) {
        EXPECT_EQ(test_val.first.at(pos), data.at(pos));
      }
      EXPECT_EQ(test_val.second.size(), n_rows);
      for (uint16_t pos = 0; pos < n_rows; pos++) {
        EXPECT_EQ(test_val.second.at(pos), parity.at(pos));
      }
    }
  }
}

TEST(test_streaming_code_helper, test_get_recoverable_data_burst_v_recover)
{
  for (uint16_t timeslot = 6; timeslot < 70; timeslot++) {
    for (uint16_t w = 8; w < 9; w += 8) {
      uint16_t delay = 3;
      uint16_t n_data_p_f = 8;
      uint16_t n_par_p_f = 8;
      auto num_frames = num_frames_for_delay(delay);
      uint16_t timeslot = 2 * num_frames - 1;
      auto n_cols = uint16_t(n_data_p_f * num_frames);
      auto n_rows = uint16_t(n_par_p_f * num_frames);
      CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
      StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
      auto retained_parities_by_mem = helper.
          get_retained_parities_zero_by_memory(timeslot);
      vector<bool> frame_erased_orig = { false, false, false, false, true, true,
          false};
      uint16_t shift = (timeslot - 6) % num_frames;
      vector<bool> frame_erased(7, false);
      for (uint16_t pos = 0; pos < 7; pos++) {
        frame_erased.at(pos) = frame_erased_orig.at((pos + shift) % num_frames);
      }
      StreamingCodeTheoretical streamingCodeTheoretical(delay);
      vector<bool> erased(n_rows + n_cols, false);
      vector<bool> parity = helper.get_retained_parities_zero_by_memory(timeslot);
      for (uint16_t t = timeslot - 2; t <= timeslot - 1;
          t++) {
        auto endpointsCol = cols_of_frame(t, delay, codingMatrixInfo);
        for (uint16_t i = endpointsCol.first; i <= endpointsCol.second; i++) {
          erased.at(i) = true;
        }
        auto endpointsRow = rows_of_frame(t, delay, codingMatrixInfo);
        for (uint16_t i = endpointsRow.first; i <= endpointsRow.second; i++) {
          erased.at(n_cols + i) = true;
          parity.at(i) = false;
        }
      }
      for (uint16_t t = 0; t < num_frames; t++) {
        if (t != (timeslot % num_frames)) {
          auto endpointsRow = rows_of_frame(t, delay, codingMatrixInfo);
          for (uint16_t i = endpointsRow.first; i <= endpointsRow.second; i++) {
            parity.at(i) = false;
          }
        }
      }
      vector<bool> data(n_cols, false);
      auto endpoints = cols_of_frame(timeslot - 2, delay, codingMatrixInfo);
      for (uint16_t i = endpoints.first; i <= endpoints.second; i++) {
        if (!helper.get_positions_of_us().at(i)) { data.at(i) = true; }
      }
      endpoints = cols_of_frame(timeslot - 1, delay, codingMatrixInfo);
      for (uint16_t i = endpoints.first; i <= endpoints.second; i++) {
        if (!helper.get_positions_of_us().at(i)) { data.at(i) = true; }
      }
      auto test_val = helper.get_recoverable_data(erased, timeslot);
      EXPECT_EQ(test_val.first.size(), n_cols);
      for (uint16_t pos = 0; pos < n_cols; pos++) {
        EXPECT_EQ(test_val.first.at(pos), data.at(pos));
      }
      EXPECT_EQ(test_val.second.size(), n_rows);
      for (uint16_t pos = 0; pos < n_rows; pos++) {
        EXPECT_EQ(test_val.second.at(pos), parity.at(pos));
      }
    }
  }
}

TEST(test_streaming_code_helper, test_get_recoverable_data_burst_not_recovered)
{
  for (uint16_t timeslot = 6; timeslot < 70; timeslot++) {
    for (uint16_t w = 8; w < 9; w += 8) {
      uint16_t delay = 3;
      uint16_t n_data_p_f = 8;
      uint16_t n_par_p_f = 4;
      auto num_frames = num_frames_for_delay(delay);
      uint16_t timeslot = 2 * num_frames - 1;
      auto n_cols = uint16_t(n_data_p_f * num_frames);
      auto n_rows = uint16_t(n_par_p_f * num_frames);
      CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
      StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
      auto retained_parities_by_mem = helper.
          get_retained_parities_zero_by_memory(timeslot);
      vector<bool> frame_erased_orig = { false, false, false, true, true, false,
          false};
      uint16_t shift = (timeslot - 6) % num_frames;
      vector<bool> frame_erased(7, false);
      for (uint16_t pos = 0; pos < 7; pos++) {
        frame_erased.at(pos) = frame_erased_orig.at((pos + shift) % num_frames);
      }
      StreamingCodeTheoretical streamingCodeTheoretical(delay);
      vector<bool> erased(n_rows + n_cols, false);
      vector<bool> parity(n_cols, false);
      for (uint16_t t = timeslot - 3; t <= timeslot - 2; t++) {
        auto endpointsCol = cols_of_frame(t, delay, codingMatrixInfo);
        for (uint16_t i = endpointsCol.first; i <= endpointsCol.second; i++) {
          erased.at(i) = true;
        }
        auto endpointsRow = rows_of_frame(t, delay, codingMatrixInfo);
        for (uint16_t i = endpointsRow.first; i <= endpointsRow.second; i++) {
          erased.at(n_cols + i) = true;
        }
      }
      vector<bool> data(n_cols, false);
      auto test_val = helper.get_recoverable_data(erased, timeslot);
      EXPECT_EQ(test_val.first.size(), n_cols);
      for (uint16_t pos = 0; pos < n_cols; pos++) {
        EXPECT_EQ(test_val.first.at(pos), data.at(pos));
      }
      EXPECT_EQ(test_val.second.size(), n_rows);
      for (uint16_t pos = 0; pos < n_rows; pos++) {
        EXPECT_EQ(test_val.second.at(pos), parity.at(pos));
      }
    }
  }
}

TEST(test_streaming_code_helper, test_get_retained_parity_info)
{
  for (uint16_t trial = 0; trial < 1000; trial++) {
    for (uint16_t n_par_p_f = 1; n_par_p_f < 4; n_par_p_f++) {
      for (uint16_t n_data_p_f = 1; n_data_p_f < 4; n_data_p_f++) {
        for (uint16_t delay = 0; delay < 4; delay++) {
          for (uint16_t w = 8; w < 33; w += 8) {
            auto num_frames = num_frames_for_delay(delay);
            auto n_cols = uint16_t(n_data_p_f * num_frames);
            auto n_rows = uint16_t(n_par_p_f * num_frames);
            if (n_cols + n_rows >= 256) { continue; }
            CodingMatrixInfo codingMatrixInfo { n_rows , n_cols, w };
            StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
            for (uint16_t timeslot = 0; timeslot < 4 * num_frames; timeslot++) {
              auto retained_parities_by_mem = helper.
                  get_retained_parities_zero_by_memory(timeslot);
              vector<bool> erased = get_missing((uint8_t) (n_cols+ n_rows),
                  (uint8_t) (rand() % (n_cols + n_rows)));
              vector<bool> unusable_parities_by_frame = get_missing((uint8_t)
                  num_frames, (uint8_t) (rand() % num_frames));
              vector<bool> unusable_parities(n_rows, false);
              for (uint16_t j = 0; j < num_frames; j++) {
                auto endpoints = rows_of_frame(j, delay, codingMatrixInfo);
                for (uint16_t i = endpoints.first; i <= endpoints.second; i++) {
                  unusable_parities.at(i) = unusable_parities_by_frame.at(j);
                }
              }
              vector<bool> retained_parities;
              for (uint16_t pos = 0; pos < n_rows; pos++) {
                retained_parities.push_back(!erased.at(n_cols + pos) and
                    !unusable_parities.at(pos) and
                    retained_parities_by_mem.at(pos));
              }
              auto test_val = helper.get_retained_parity_info(erased, timeslot,
                  unusable_parities_by_frame, vector<uint16_t>(n_rows, 1000));
              EXPECT_EQ(test_val.size(), n_rows);
              for (uint16_t pos = 0; pos < n_rows; pos++) {
                EXPECT_EQ(test_val.at(pos), retained_parities.at(pos));
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
