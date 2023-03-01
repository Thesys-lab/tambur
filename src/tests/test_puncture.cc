#include "gtest/gtest.h"
#include <cmath>

#include "puncture.hh"

using namespace std;

TEST(test_puncture, test_get_puncture)
{
  for (uint16_t n_par_p_f = 1; n_par_p_f < 4; n_par_p_f++) {
    for (uint16_t n_data_p_f = 1; n_data_p_f < 4; n_data_p_f++) {
      for (uint16_t delay = 0; delay < 7; delay++) {
        for (uint16_t w = 8; w < 33; w += 8) {
          auto num_frames = num_frames_for_delay(delay);
          CodingMatrixInfo codingMatrixInfo { uint16_t(n_par_p_f * num_frames),
              uint16_t(n_data_p_f * num_frames), w };
          optional<Matrix<int>> mat;
          mat.emplace(codingMatrixInfo.n_rows,
              codingMatrixInfo.n_cols);
          set_cauchy_matrix(mat.value(), codingMatrixInfo, delay);
          for (uint16_t val = 0; val < uint16_t(pow(2, num_frames)); val++) {
            auto retained_frames = get_retained_frames(val, num_frames);
            uint16_t num_ret_frames = 0;
            for (auto el : retained_frames) { if (el) { num_ret_frames++; } }
            optional<Matrix<int>> puncture;
            auto dim = get_puncture_dim(delay, mat->max_rows(),
                retained_frames);
            if (dim > 0) {
              puncture.emplace(dim, mat->max_cols());
              get_puncture<int>(codingMatrixInfo, delay, mat, puncture.value(),
                  retained_frames);
            }
            EXPECT_EQ(puncture.has_value(),
                num_ret_frames > 0);
            if (num_ret_frames == 0) { continue; }
            EXPECT_EQ(puncture->rows(),
                num_ret_frames*n_par_p_f);
            EXPECT_EQ(puncture->cols(),
                mat->cols());
            uint16_t rel_frame = 0;
            for (uint16_t frame_num = 0; frame_num < num_frames; frame_num++) {
              auto rows = rows_of_frame(frame_num, delay, codingMatrixInfo);
              bool retained = retained_frames.at(frame_num);
              if (retained) {
                for (uint16_t row = rows.first; row <= rows.second;
                    row ++) {
                  for (uint16_t col = 0; col < mat->cols(); col++) {
                    auto test_val = puncture->at(
                        row- rel_frame * n_par_p_f, col);
                    EXPECT_EQ(test_val, mat->at(row, col));
                  }
                }
              }
              else { rel_frame++; }
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
