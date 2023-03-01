#include "gtest/gtest.h"
#include <cmath>

#include "helpers.hh"
#include "serialization.hh"
#include "multi_frame_fec_helpers.hh"

using namespace std;

TEST(test_multi_frame_fec_helpers, test_zero_by_frame_zero_delay)
{
  for (uint16_t n_rows = 1; n_rows < 5; n_rows++) {
    for (uint16_t n_cols = 1; n_cols < 5; n_cols ++) {
      for (uint16_t w = 8; w < 33; w += 8) {
        for (uint16_t row = 0; row < n_rows; row++) {
          for (uint16_t col = 0; col < n_cols; col++) {
            EXPECT_FALSE(zero_by_frame(Position {row, col},
                CodingMatrixInfo {n_rows, n_cols, w}, 0));
          }
        }
      }
    }
  }
}

TEST(test_multi_frame_fec_helpers, test_zero_by_frame) {
  for (uint16_t delay = 1; delay < 7; delay++) {
    uint16_t n_frames = num_frames_for_delay(delay);
    for (uint16_t n_rows_p_f = 1; n_rows_p_f < 5; n_rows_p_f++) {
      uint16_t n_rows = n_rows_p_f * n_frames;
      for (uint16_t n_cols_p_f = 1; n_cols_p_f < 5; n_cols_p_f ++) {
        uint16_t n_cols = n_cols_p_f * n_frames;
        for (uint16_t w = 8; w < 33; w += 8) {
          for (uint16_t row = 0; row < n_rows; row++) {
            for (uint16_t col = 0; col < n_cols; col++) {
              auto frame_num_mod_n_frames = row / n_rows_p_f;
              auto encode_frame_mod_n_frames = col / n_cols_p_f;
              vector<uint16_t> mask;
              for (uint16_t j = 0; j < delay; j++) {
                mask.push_back((frame_num_mod_n_frames + 1 + j) % n_frames);
              }
              bool is_zero = false;
              for (uint16_t el : mask) {
                if (el == encode_frame_mod_n_frames) { is_zero = true; }
              }
              EXPECT_EQ(is_zero, zero_by_frame(Position {row, col},
                  CodingMatrixInfo {n_rows, n_cols, w}, delay));
            }
          }
        }
      }
    }
  }
}

TEST(test_multi_frame_fec_helpers, test_rows_of_frame)
{
  for (uint16_t delay = 0; delay < 7; delay++) {
    uint16_t n_frames = num_frames_for_delay(delay);
    for (uint16_t n_rows_p_f = 1; n_rows_p_f < 5; n_rows_p_f++) {
      uint16_t n_rows = n_rows_p_f * n_frames;
      for (uint16_t n_cols_p_f = 1; n_cols_p_f < 5; n_cols_p_f ++) {
        uint16_t n_cols = n_cols_p_f * n_frames;
        for (uint16_t w = 8; w < 33; w += 8) {
          for (uint16_t frame_num = 0; frame_num < 10*n_frames; frame_num++) {
            auto start = (frame_num % n_frames) * n_rows_p_f;
            pair<uint16_t, uint16_t> vals(start, start + n_rows_p_f - 1);
            auto test_rows = rows_of_frame(frame_num, delay,
                CodingMatrixInfo {n_rows, n_cols, w});
            EXPECT_EQ(vals, test_rows);
          }
        }
      }
    }
  }
}

TEST(test_multi_frame_fec_helpers, test_cols_of_frame)
{
  for (uint16_t delay = 0; delay < 7; delay++) {
    uint16_t n_frames = num_frames_for_delay(delay);
    for (uint16_t n_rows_p_f = 1; n_rows_p_f < 5; n_rows_p_f++) {
      uint16_t n_rows = n_rows_p_f * n_frames;
      for (uint16_t n_cols_p_f = 1; n_cols_p_f < 5; n_cols_p_f ++) {
        uint16_t n_cols = n_cols_p_f * n_frames;
        for (uint16_t w = 8; w < 33; w += 8) {
          for (uint16_t frame_num = 0; frame_num < 10*n_frames; frame_num++) {
            auto start = (frame_num % n_frames) * n_cols_p_f;
            pair<uint16_t, uint16_t> vals(start, start + n_cols_p_f - 1);
            auto test_cols = cols_of_frame(frame_num, delay,
                CodingMatrixInfo {n_rows, n_cols, w});
            EXPECT_EQ(vals, test_cols);
          }
        }
      }
    }
  }
}

TEST(test_multi_frame_fec_helpers, test_frame_of_row)
{
  for (uint16_t delay = 0; delay < 7; delay++) {
    uint16_t n_frames = num_frames_for_delay(delay);
    for (uint16_t n_rows_p_f = 1; n_rows_p_f < 5; n_rows_p_f++) {
      uint16_t n_rows = n_rows_p_f * n_frames;
      for (uint16_t n_cols_p_f = 1; n_cols_p_f < 5; n_cols_p_f ++) {
        uint16_t n_cols = n_cols_p_f * n_frames;
        for (uint16_t w = 8; w < 33; w += 8) {
          for (uint16_t row = 0; row < n_rows; row++) {
            EXPECT_EQ(row / n_rows_p_f, frame_of_row(
                row, delay, CodingMatrixInfo {n_rows, n_cols, w}));
          }
          for (uint16_t row = n_rows; row < 2 * n_rows; row++) {
            try {
              frame_of_row(row, delay,
                  CodingMatrixInfo {n_rows, n_cols, w});
              EXPECT_TRUE(false);
            }
            catch (...) {
              EXPECT_TRUE(true);
            }
          }
        }
      }
    }
  }
}

TEST(test_multi_frame_fec_helpers, test_frame_of_col)
{
  for (uint16_t delay = 0; delay < 7; delay++) {
    uint16_t n_frames = num_frames_for_delay(delay);
    for (uint16_t n_rows_p_f = 1; n_rows_p_f < 5; n_rows_p_f++) {
      uint16_t n_rows = n_rows_p_f * n_frames;
      for (uint16_t n_cols_p_f = 1; n_cols_p_f < 5; n_cols_p_f ++) {
        uint16_t n_cols = n_cols_p_f * n_frames;
        for (uint16_t w = 8; w < 33; w += 8) {
          for (uint16_t col = 0; col < n_cols; col++) {
            EXPECT_EQ(col / n_cols_p_f, frame_of_col(
                col, delay, CodingMatrixInfo {n_rows, n_cols, w}));
          }
          for (uint16_t col = n_cols; col < 2 * n_cols; col++) {
            try {
              frame_of_col(col, delay,
                  CodingMatrixInfo {n_rows, n_cols, w});
              EXPECT_TRUE(false);
            }
            catch (...) {
              EXPECT_TRUE(true);
            }
          }
        }
      }
    }
  }
}

TEST(test_multi_frame_fec_helpers, test_get_retained_frames)
{
  for (uint16_t n_frames = 1; n_frames < 4; n_frames++) {
    for (uint16_t val = 0; val < uint16_t(pow(2,n_frames)); val++) {
      vector<bool> test_retained_frames = get_retained_frames(val, n_frames);
      vector<bool> retained_frames;
      uint16_t val_copy = val;
      for (uint16_t pos = 0; pos < n_frames; pos++) {
        retained_frames.push_back(val_copy % 2);
        val_copy /= 2;
      }
      EXPECT_EQ(test_retained_frames.size(), n_frames);
      for (uint16_t pos = 0; pos < n_frames; pos++) {
        EXPECT_EQ(retained_frames.at(pos), test_retained_frames.at(pos));
      }
    }
  }
}

TEST(test_multi_frame_fec_helpers, test_reorder_to_matrix_positions)
{
  srand(0);
  for (uint16_t trial = 0; trial < 1000; trial++) {
    for (uint16_t num_frames = 1; num_frames < 10; num_frames++) {
      for (uint16_t timeslot = 0; timeslot < 10*num_frames; timeslot++) {
        vector<bool> initial_vals = get_missing((uint8_t) num_frames,
                      (uint8_t) (rand() % num_frames + 1));
        vector<bool> test_vals = reorder_to_matrix_positions<bool>(timeslot,
            initial_vals, num_frames);
        EXPECT_EQ(test_vals.size(), num_frames);
        if (num_frames == 1 or ( num_frames == 2 and
            (timeslot % num_frames) == 1 )) {
          EXPECT_EQ(initial_vals.at(0), test_vals.at(0));
        }
        else if (num_frames == 2) {
          EXPECT_EQ(initial_vals.at(1), test_vals.at(0));
          EXPECT_EQ(initial_vals.at(0), test_vals.at(1));
        }
        else {
          vector<bool> vals(num_frames, true);
          for (uint16_t j = timeslot; j < timeslot + num_frames; j++) {
            vals.at(j % num_frames) =
                initial_vals.at((num_frames - 1 + j - timeslot) % num_frames);
          }
          for (uint16_t j = 0; j < num_frames; j++) {
            EXPECT_EQ(vals.at(j), test_vals.at(j));
          }
        }
      }
    }
  }
}


TEST(test_multi_frame_fec_helpers, test_convert_string_to_FEC_input)
{
  for (uint16_t j = 0; j < 4*sizeof(long); j++) {
    string s, test_val;
    for (uint16_t i = 0; i < j; i++) {
      s.push_back(char(i));
      test_val.push_back(char(i));
    }
    convert_string_to_FEC_input(test_val);
    while ((s.size() % sizeof(long)) > 0) { s.push_back('0'); }
    EXPECT_EQ(test_val, s);
  }
}

TEST(test_multi_frame_fec_helpers, test_convert_strings_to_FEC_inputs)
{
  for (uint16_t total = 0; total < 4; total++) {
    vector<string> s, test_vals;
    for (uint16_t l = 0; l < total; l++) {
      for (uint16_t j = 1; j < 4*sizeof(long); j++) {
        string tmp,tmp_test;
        for (uint16_t i = 0; i < j; i++) {
          tmp.push_back(char(i));
          tmp_test.push_back(char(i));
        }
        convert_string_to_FEC_input(tmp);
        s.push_back(tmp);
        test_vals.push_back(tmp_test);
      }
    }
    convert_strings_to_FEC_inputs(test_vals);
    EXPECT_EQ(test_vals, s);
  }
}

TEST(test_multi_frame_fec_helpers, test_convert_sizes_to_FEC_input)
{
  for (uint16_t num_sizes = 0; num_sizes < 10; num_sizes++) {
    vector<uint64_t> sizes;
    vector<string> inputs;
    for (uint8_t pos = 0; pos < num_sizes; pos++) {
      uint64_t size = uint64_t(pos + 20);
      sizes.push_back(size);
      string size_string = put_number(size);
      inputs.push_back(size_string);
    }
    EXPECT_EQ(inputs, convert_sizes_to_FEC_input(sizes));
  }
}

int main(int argc, char * argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
