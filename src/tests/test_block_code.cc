#include "gtest/gtest.h"
#include <chrono>
#include <cmath>

#include "block_code.hh"
#include "helpers.hh"

using namespace std;

TEST(test_block_code, test_place_payload_get_row)
{
  for (uint16_t n_par_p_f = 1; n_par_p_f < 4; n_par_p_f++) {
    for (uint16_t n_data_p_f = 1; n_data_p_f < 4; n_data_p_f++) {
      for (uint16_t delay = 0; delay < 4; delay++) {
        for (uint16_t w = 8; w < 33; w += 8) {
          for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
            for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
              vector<char> default_val(w * packet_size, char(0));
              auto num_frames = num_frames_for_delay(delay);
              auto n_cols = uint16_t(n_data_p_f * num_frames);
              auto n_rows = uint16_t(n_par_p_f * num_frames);
              vector<vector<char>> data;
              for (uint16_t j = 0; j < n_cols; j++) {
                vector<char> data_row;
                for (uint16_t l = 0; l < w * packet_size; l++) {
                  data_row.push_back(char(j));
                }
                data.push_back(data_row);
              }
              for (uint16_t j = 0; j < n_rows; j++) {
                vector<char> parity_row;
                for (uint16_t l = 0; l < w * packet_size; l++) {
                  parity_row.push_back(char(j + n_cols));
                }
                data.push_back(parity_row);
              }
              CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
              BlockCode blockCode(codingMatrixInfo, delay,
                  { 1 , 1 }, packet_size, bitmatrix > 0);
              for (uint16_t pos = 0; pos < n_cols; pos++) {
                EXPECT_EQ(blockCode.get_row(pos, false).size(),
                    default_val.size());
                for (uint16_t i = 0; i < default_val.size(); i++) {
                  EXPECT_EQ(blockCode.get_row(pos, false).at(i),
                    default_val.at(i));
                }
                blockCode.place_payload(pos, false, data.at(pos));
                EXPECT_FALSE(blockCode.get_erased().at(pos));
                EXPECT_EQ(blockCode.get_row(pos, false).size(),
                    default_val.size());
                EXPECT_EQ(blockCode.get_row(pos, false).size(),
                    data.at(pos).size());
                for (uint16_t i = 0; i < default_val.size(); i++) {
                  EXPECT_EQ(blockCode.get_row(pos, false).at(i),
                    data.at(pos).at(i));
                }
              }
              for (uint16_t pos = 0; pos < n_rows; pos++) {
                EXPECT_EQ(blockCode.get_row(pos, true).size(),
                    default_val.size());
                for (uint16_t i = 0; i < default_val.size(); i++) {
                  EXPECT_EQ(blockCode.get_row(pos, true).at(i),
                    default_val.at(i));
                }
                blockCode.place_payload(pos, true, data.at(pos + n_cols));
                EXPECT_FALSE(blockCode.get_erased().at(pos + n_cols));
                EXPECT_EQ(blockCode.get_row(pos, true).size(),
                    default_val.size());
                EXPECT_EQ(blockCode.get_row(pos, true).size(),
                    data.at(pos + n_cols).size());
                for (uint16_t i = 0; i < default_val.size(); i++) {
                  EXPECT_EQ(blockCode.get_row(pos, true).at(i),
                    data.at(pos + n_cols).at(i));
                }
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_block_code, test_update_timeslot)
{
    for (uint16_t n_par_p_f = 1; n_par_p_f < 4; n_par_p_f++) {
    for (uint16_t n_data_p_f = 1; n_data_p_f < 4; n_data_p_f++) {
      for (uint16_t delay = 0; delay < 4; delay++) {
        for (uint16_t w = 8; w < 33; w += 8) {
          for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
            for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
              auto num_frames = num_frames_for_delay(delay);
              for (uint16_t timeslot = 0; timeslot < 3*num_frames; timeslot++) {
                vector<char> default_val(w * packet_size, char(0));
                auto n_cols = uint16_t(n_data_p_f * num_frames);
                auto n_rows = uint16_t(n_par_p_f * num_frames);
                vector<vector<char>> data;
                for (uint16_t j = 0; j < n_cols; j++) {
                  vector<char> data_row;
                  for (uint16_t l = 0; l < w * packet_size; l++) {
                    data_row.push_back(char(j+1));
                  }
                  data.push_back(data_row);
                }
                for (uint16_t j = 0; j < n_rows; j++) {
                  vector<char> parity_row;
                  for (uint16_t l = 0; l < w * packet_size; l++) {
                    parity_row.push_back(char(j + n_cols+1));
                  }
                  data.push_back(parity_row);
                }
                CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
                BlockCode blockCode(codingMatrixInfo, delay,
                    { 1 , 1 }, packet_size, bitmatrix > 0);
                for (uint16_t pos = 0; pos < n_cols; pos++) {
                  blockCode.place_payload(pos, false, data.at(pos));
                }
                for (uint16_t pos = 0; pos < n_rows; pos++) {
                  blockCode.place_payload(pos, true, data.at(pos + n_cols));
                }
                for (uint16_t time = 0; time < timeslot; time++) {
                  blockCode.update_timeslot(time);
                  auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
                  for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                    EXPECT_TRUE(blockCode.get_erased().at(pos));
                    blockCode.place_payload(pos, false, data.at(pos));
                    EXPECT_FALSE(blockCode.get_erased().at(pos));
                  }
                  auto rows_frame = rows_of_frame(time, delay, codingMatrixInfo);
                  for (uint16_t pos = rows_frame.first; pos <= rows_frame.second; pos++) {
                    EXPECT_TRUE(blockCode.get_erased().at(pos + n_cols));
                    blockCode.place_payload(pos, true, data.at(n_cols + pos));
                    EXPECT_FALSE(blockCode.get_erased().at(pos + n_cols));
                  }
                }
                blockCode.update_timeslot(timeslot);
                for (uint16_t t = timeslot; t < timeslot + num_frames; t++) {
                  auto endpoints = cols_of_frame(t, delay, codingMatrixInfo);
                  for (uint16_t pos = endpoints.first; pos <= endpoints.second;
                      pos++) {
                    EXPECT_EQ(blockCode.get_row(pos, false).size(),
                        default_val.size());
                    for (uint16_t i = 0; i < default_val.size(); i++) {
                      EXPECT_EQ(blockCode.get_row(pos, false).at(i),
                        (t != timeslot? data.at(pos) : default_val).at(i));
                      EXPECT_EQ(blockCode.get_erased().at(pos), (t == timeslot));
                    }
                  }
                  endpoints = rows_of_frame(t, delay, codingMatrixInfo);
                  for (uint16_t pos = endpoints.first; pos <= endpoints.second;
                      pos++) {
                    EXPECT_EQ(blockCode.get_row(pos, true).size(),
                        default_val.size());
                    for (uint16_t i = 0; i < default_val.size(); i++) {
                      EXPECT_EQ(blockCode.get_row(pos, true).at(i),
                        (t != timeslot? data.at(pos + n_cols) : default_val).at(i));
                      EXPECT_EQ(blockCode.get_erased().at(pos + n_cols),
                          (t == timeslot));
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
}

TEST(test_block_code, test_coded_payload)
{
  for (uint16_t n_par_p_f = 1; n_par_p_f < 4; n_par_p_f++) {
    for (uint16_t n_data_p_f = 1; n_data_p_f < 4; n_data_p_f++) {
      for (uint16_t delay = 0; delay < 4; delay++) {
        for (uint16_t w = 8; w < 33; w += 8) {
          for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
            for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
              auto num_frames = num_frames_for_delay(delay);
              for (uint16_t timeslot = 0; timeslot < 3*num_frames; timeslot++) {
                vector<char> default_val(w * packet_size, char(0));
                auto n_cols = uint16_t(n_data_p_f * num_frames);
                auto n_rows = uint16_t(n_par_p_f * num_frames);
                vector<vector<char>> data;
                for (uint16_t j = 0; j < n_cols; j++) {
                  vector<char> data_row;
                  for (uint16_t l = 0; l < w * packet_size; l++) {
                    data_row.push_back(char(j+1));
                  }
                  data.push_back(data_row);
                }
                for (uint16_t j = 0; j < n_rows; j++) {
                  vector<char> parity_row;
                  for (uint16_t l = 0; l < w * packet_size; l++) {
                    parity_row.push_back(char(j + n_cols+1));
                  }
                  data.push_back(parity_row);
                }
                CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
                BlockCode blockCode(codingMatrixInfo, delay,
                    { 1 , 1 }, packet_size, bitmatrix > 0);
                for (uint16_t pos = 0; pos < n_cols; pos++) {
                  blockCode.place_payload(pos, false, data.at(pos));
                }
                for (uint16_t pos = 0; pos < n_rows; pos++) {
                  blockCode.place_payload(pos, true, data.at(pos + n_cols));
                }
                for (uint16_t time = 0; time < timeslot; time++) {
                  blockCode.update_timeslot(time);
                  auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
                  for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                    blockCode.place_payload(pos, false, data.at(pos));
                  }
                  auto rows_frame = rows_of_frame(time, delay, codingMatrixInfo);
                  for (uint16_t pos = rows_frame.first; pos <= rows_frame.second; pos++) {
                    blockCode.place_payload(pos, true, data.at(n_cols + pos));
                  }
                }
                blockCode.update_timeslot(timeslot);
                auto endpoints = rows_of_frame(timeslot, delay, codingMatrixInfo);
                Matrix<char> mat(n_par_p_f, packet_size * w);
                for (uint16_t row  = 0; row < n_par_p_f; row++) {
                  for (uint16_t col = 0; col < packet_size * w; col++) {
                    mat.at(row, col) = char(row * packet_size * w + col);
                  }
                }
                blockCode.coded_payload(endpoints.first, endpoints.second, mat);
                for (uint16_t pos = endpoints.first; pos <= endpoints.second;
                    pos++) {
                  EXPECT_EQ(blockCode.get_row(pos, true).size(), packet_size * w);
                  for (uint16_t i = 0; i < packet_size * w; i++) {
                    EXPECT_EQ(blockCode.get_row(pos, true).at(i),
                      mat.at(pos - endpoints.first, i));
                  }
                  EXPECT_FALSE(blockCode.get_erased().at(pos + n_cols));
                }
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_block_code, test_encode)
{
  srand(0);
  float total = 0;
  float intersection = 0;
  for (uint16_t n_par_p_f = 1; n_par_p_f < 4; n_par_p_f++) {
    for (uint16_t n_data_p_f = 1; n_data_p_f < 4; n_data_p_f++) {
      for (uint16_t delay = 0; delay < 4; delay++) {
        for (uint16_t w = 8; w < 33; w += 8) {
          if (w == 24) { continue; }
          for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
            for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
              auto num_frames = num_frames_for_delay(delay);
              for (uint16_t timeslot = 0; timeslot < 3*num_frames; timeslot++) {
                total += n_par_p_f;
                vector<char> default_val(w * packet_size, char(0));
                auto n_cols = uint16_t(n_data_p_f * num_frames);
                auto n_rows = uint16_t(n_par_p_f * num_frames);
                vector<vector<char>> data;
                for (uint16_t j = 0; j < n_cols + n_rows; j++) {
                  vector<char> data_row;
                  for (uint16_t l = 0; l < w * packet_size; l++) {
                    data_row.push_back(char(j+1));
                  }
                  data.push_back(data_row);
                }
                CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
                BlockCode encodeBlockCode(codingMatrixInfo, delay,
                    { 1 , 1 }, packet_size, bitmatrix > 0);
                for (uint16_t pos = 0; pos < n_cols; pos++) {
                  encodeBlockCode.place_payload(pos, false, data.at(pos));
                }
                for (uint16_t pos = 0; pos < n_rows; pos++) {
                  encodeBlockCode.place_payload(pos, true, data.at(pos + n_cols));
                }
                for (uint16_t time = 0; time < timeslot; time++) {
                  encodeBlockCode.update_timeslot(time);
                  auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
                  for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                    encodeBlockCode.place_payload(pos, false, data.at(pos));
                  }
                  auto rows_frame = rows_of_frame(time, delay, codingMatrixInfo);
                  for (uint16_t pos = rows_frame.first; pos <= rows_frame.second; pos++) {
                    encodeBlockCode.place_payload(pos, true, data.at(n_cols + pos));
                  }
                }
                encodeBlockCode.update_timeslot(timeslot);
                auto endpoints = cols_of_frame(timeslot, delay, codingMatrixInfo);
                for (uint16_t pos = endpoints.first; pos <= endpoints.second;
                    pos++) {
                  encodeBlockCode.place_payload(pos, false, data.at(pos));
                }
                encodeBlockCode.encode();
                auto par_endpoints = rows_of_frame(timeslot, delay,
                    codingMatrixInfo);
                for (uint16_t pos = par_endpoints.first; pos <=
                    par_endpoints.second; pos++) {
                  const auto val = encodeBlockCode.get_row(pos, true);
                  EXPECT_EQ(val.size(), packet_size * w);
                  vector<char> payload;
                  for (uint16_t j = 0; j < val.size(); j++) {
                    payload.push_back(val.at(j));
                  }
                  bool intersect = false;
                  for (uint16_t j = 0; j < data.size(); j++) {
                    if (data.at(j).front() == payload.front()) {
                      intersect = true;
                    }
                  }
                  intersection += intersect;
                }
              }
            }
          }
        }
      }
    }
  }
  EXPECT_LE(intersection / total, .95);
}

TEST(test_block_code, test_encode_decode_0_delay)
{
  srand(0);
  for (uint16_t trial = 0; trial < 20; trial++) {
    for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
      for (uint16_t w = 8; w < 33; w += 8) {
        if (w == 24) { continue; }
        for (uint16_t n_data_p_f = 1; n_data_p_f < 5; n_data_p_f++) {
          for (uint16_t n_par_p_f = n_data_p_f; n_par_p_f < 7; n_par_p_f++) {
            for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
              uint16_t delay = 0;
              auto num_frames = num_frames_for_delay(delay);
              for (uint16_t timeslot = 0; timeslot < 3*num_frames; timeslot++) {
                for (uint16_t u = 0; u < 2; u++) {
                  vector<char> default_val(w * packet_size, char(0));
                  auto n_cols = uint16_t(n_data_p_f * num_frames);
                  auto n_rows = uint16_t(n_par_p_f * num_frames);
                  vector<vector<char>> data;
                  for (uint16_t j = 0; j < n_cols; j++) {
                    vector<char> data_row;
                    for (uint16_t l = 0; l < w * packet_size; l++) {
                      data_row.push_back(char(3*j+1));
                    }
                    data.push_back(data_row);
                  }
                  CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
                  BlockCode encodeBlockCode(codingMatrixInfo, delay,
                      { 1 , u }, packet_size, bitmatrix > 0);
                  BlockCode decodeBlockCode(codingMatrixInfo, delay,
                      { 1 , u }, packet_size, bitmatrix > 0);
                  for (uint16_t time = 0; time < timeslot; time++) {
                    encodeBlockCode.update_timeslot(time);
                    decodeBlockCode.update_timeslot(time);
                    auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
                    for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                      encodeBlockCode.place_payload(pos, false, data.at(pos));
                      decodeBlockCode.place_payload(pos, false, data.at(pos));
                    }
                  }
                  encodeBlockCode.update_timeslot(timeslot);
                  decodeBlockCode.update_timeslot(timeslot);
                  auto endpoints = cols_of_frame(timeslot, delay, codingMatrixInfo);
                  auto missing =  get_missing(n_data_p_f, rand() % (n_data_p_f + 1));
                  uint16_t num_missing = 0;
                  for (uint16_t pos = endpoints.first; pos <= endpoints.second;
                      pos++) {
                    if (!missing.at(pos -endpoints.first)) {
                      decodeBlockCode.place_payload(pos, false, data.at(pos));
                    }
                    else { num_missing++; }
                    encodeBlockCode.place_payload(pos, false, data.at(pos));
                  }
                  encodeBlockCode.encode();
                  auto par_endpoints = rows_of_frame(timeslot, delay,
                      codingMatrixInfo);
                  auto missing_par = get_missing(n_par_p_f, rand() %
                      (n_par_p_f-num_missing +1));
                  for (uint16_t pos = par_endpoints.first; pos <=
                      par_endpoints.second; pos++) {
                    if (!missing_par.at(pos - par_endpoints.first)) {
                      const auto val = encodeBlockCode.get_row(pos, true);
                      vector<char> payload;
                      for (uint16_t j = 0; j < val.size(); j++) {
                        payload.push_back(val.at(j));
                      }
                      decodeBlockCode.place_payload(pos, true, payload);
                    }
                  }
                  if (num_missing == 0) {
                    EXPECT_FALSE(decodeBlockCode.can_recover());
                    continue;
                  }
                  else {
                    EXPECT_TRUE(decodeBlockCode.can_recover());
                  }
                  auto decoded = decodeBlockCode.decode();
                  uint16_t num_decoded = 0;
                  for (auto el : decoded) { num_decoded += el; }
                  for (uint16_t pos = endpoints.first; pos <= endpoints.second;
                      pos++) {
                    EXPECT_EQ(decodeBlockCode.get_row(pos, false).size(),
                        packet_size * w);
                    for (uint16_t i = 0; i < packet_size * w; i++) {
                      EXPECT_EQ(decodeBlockCode.get_row(pos, false).at(i),
                        data.at(pos).at(i));
                    }
                    EXPECT_FALSE(decodeBlockCode.get_erased().at(pos));
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

TEST(test_block_code, test_encode_decode_within_frame_single_loss)
{
  for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
    for (uint16_t w = 8; w < 33; w += 8) {
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 1; n_data_p_f < 5; n_data_p_f++) {
        for (uint16_t n_par_p_f = n_data_p_f; n_par_p_f < n_data_p_f+3; n_par_p_f++) {
          for (uint16_t delay = 0; delay < 4; delay++) {
            for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
              auto num_frames = num_frames_for_delay(delay);
              for (uint16_t timeslot = 0; timeslot < num_frames*3; timeslot++) {
                for (uint16_t u = 0; u < 2; u++) {
                  vector<char> default_val(w * packet_size, char(0));
                  auto n_cols = uint16_t(n_data_p_f * num_frames);
                  auto n_rows = uint16_t(n_par_p_f * num_frames);
                  vector<vector<char>> data;
                  for (uint16_t j = 0; j < n_cols; j++) {
                    vector<char> data_row;
                    for (uint16_t l = 0; l < w * packet_size; l++) {
                      data_row.push_back(char(3*j+1));
                    }
                    data.push_back(data_row);
                  }
                  CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
                  BlockCode encodeBlockCode(codingMatrixInfo, delay,
                      { 1 , u }, packet_size, bitmatrix > 0);
                  BlockCode decodeBlockCode(codingMatrixInfo, delay,
                      { 1 , u }, packet_size, bitmatrix > 0);
                  for (uint16_t time = 0; time < timeslot; time++) {
                    encodeBlockCode.update_timeslot(time);
                    decodeBlockCode.update_timeslot(time);
                    auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
                    for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                      encodeBlockCode.place_payload(pos, false, data.at(pos));
                      decodeBlockCode.place_payload(pos, false, data.at(pos));
                    }
                  }
                  encodeBlockCode.update_timeslot(timeslot);
                  decodeBlockCode.update_timeslot(timeslot);
                  auto endpoints = cols_of_frame(timeslot, delay, codingMatrixInfo);
                  auto missing = vector<bool>(n_data_p_f, true);
                  uint16_t num_missing = 0;
                  for (uint16_t pos = endpoints.first; pos <= endpoints.second;
                      pos++) {
                    if (!missing.at(pos -endpoints.first)) {
                      decodeBlockCode.place_payload(pos, false, data.at(pos));
                    }
                    else { num_missing++; }
                    encodeBlockCode.place_payload(pos, false, data.at(pos));
                  }
                  encodeBlockCode.encode();
                  auto par_endpoints = rows_of_frame(timeslot, delay,
                      codingMatrixInfo);
                  auto missing_par = vector<bool>(n_par_p_f, false);
                  for (uint16_t pos = par_endpoints.first; pos <=
                      par_endpoints.second; pos++) {
                    if (!missing_par.at(pos - par_endpoints.first)) {
                      const auto val = encodeBlockCode.get_row(pos, true);
                      vector<char> payload;
                      for (uint16_t j = 0; j < val.size(); j++) {
                        payload.push_back(val.at(j));
                      }
                      decodeBlockCode.place_payload(pos, true, payload);
                    }
                  }
                  if (num_missing == 0) {
                    EXPECT_FALSE(decodeBlockCode.can_recover());
                    continue;
                  }
                  else {
                    EXPECT_TRUE(decodeBlockCode.can_recover());
                  }
                  auto decoded = decodeBlockCode.decode();
                  uint16_t num_decoded = 0;
                  for (auto el : decoded) { num_decoded += el; }
                  for (uint16_t pos = endpoints.first; pos <= endpoints.second;
                      pos++) {
                    EXPECT_EQ(decodeBlockCode.get_row(pos, false).size(),
                        packet_size * w);
                    for (uint16_t i = 0; i < packet_size * w; i++) {
                      EXPECT_EQ(decodeBlockCode.get_row(pos, false).at(i),
                        data.at(pos).at(i));
                    }
                    EXPECT_FALSE(decodeBlockCode.get_erased().at(pos));
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

TEST(test_block_code, test_encode_decode_within_frame_mult_delays_single_loss)
{
  for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
    for (uint16_t w = 8; w < 33; w += 8) {
      if (w == 24) { continue; }
      uint16_t n_data_p_f = 1;
      uint16_t n_par_p_f = 1;
      for (uint16_t delay = 0; delay < 4; delay++) {
        auto num_frames = num_frames_for_delay(delay);
        for (uint16_t u = 0; u < 2; u++) {
          for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
            for (uint16_t timeslot = 0; timeslot < 3*num_frames; timeslot++) {
              vector<char> default_val(w * packet_size, char(0));
              auto n_cols = uint16_t(n_data_p_f * num_frames);
              auto n_rows = uint16_t(n_par_p_f * num_frames);
              vector<vector<char>> data;
              for (uint16_t j = 0; j < n_cols; j++) {
                vector<char> data_row;
                for (uint16_t l = 0; l < w * packet_size; l++) {
                  data_row.push_back(char(3*j+1));
                }
                data.push_back(data_row);
              }
              CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
              BlockCode encodeBlockCode(codingMatrixInfo, delay,
                  { 1 , u }, packet_size, bitmatrix > 0);
              BlockCode decodeBlockCode(codingMatrixInfo, delay,
                  { 1 , u }, packet_size, bitmatrix > 0);
              for (uint16_t time = 0; time < timeslot; time++) {
                encodeBlockCode.update_timeslot(time);
                decodeBlockCode.update_timeslot(time);
              auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
              for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                encodeBlockCode.place_payload(pos, false, data.at(pos));
                decodeBlockCode.place_payload(pos, false, data.at(pos));
              }
              }
              encodeBlockCode.update_timeslot(timeslot);
              decodeBlockCode.update_timeslot(timeslot);
              auto endpoints = cols_of_frame(timeslot, delay, codingMatrixInfo);
              auto missing = vector<bool>(n_data_p_f, true);
              uint16_t num_missing = 0;
              for (uint16_t pos = endpoints.first; pos <= endpoints.second;
                  pos++) {
                if (!missing.at(pos -endpoints.first)) {
                  decodeBlockCode.place_payload(pos, false, data.at(pos));
                }
                else { num_missing++; }
                encodeBlockCode.place_payload(pos, false, data.at(pos));
              }
              encodeBlockCode.encode();
              auto par_endpoints = rows_of_frame(timeslot, delay,
                  codingMatrixInfo);
              auto missing_par = vector<bool>(n_par_p_f, false);
              for (uint16_t pos = par_endpoints.first; pos <=
                  par_endpoints.second; pos++) {
                if (!missing_par.at(pos - par_endpoints.first)) {
                  const auto val = encodeBlockCode.get_row(pos, true);
                  vector<char> payload;
                  for (uint16_t j = 0; j < val.size(); j++) {
                    payload.push_back(val.at(j));
                  }
                  decodeBlockCode.place_payload(pos, true, payload);
                }
              }
              auto decoded = decodeBlockCode.decode();
              uint16_t num_decoded = 0;
              for (auto el : decoded) { num_decoded += el; }
              for (uint16_t pos = endpoints.first; pos <= endpoints.second;
                  pos++) {
                EXPECT_EQ(decodeBlockCode.get_row(pos, false).size(),
                    packet_size * w);
                for (uint16_t i = 0; i < packet_size * w; i++) {
                  EXPECT_EQ(decodeBlockCode.get_row(pos, false).at(i),
                    data.at(pos).at(i));
                }
                EXPECT_FALSE(decodeBlockCode.get_erased().at(pos));
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_block_code, test_encode_decode_within_frame)
{
  srand(0);
  for (uint16_t trial = 0; trial < 20; trial++) {
    for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
      for (uint16_t w = 8; w < 33; w += 8) {
        if (w == 24) { continue; }
        for (uint16_t n_data_p_f = 1; n_data_p_f < 5; n_data_p_f++) {
          for (uint16_t n_par_p_f = 1; n_par_p_f < 5; n_par_p_f++) {
            for (uint16_t delay = 0; delay < 4; delay++) {
              for (uint16_t u = 0; u < 2; u++) {
                for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
                  auto num_frames = num_frames_for_delay(delay);
                  for (uint16_t timeslot = 0; timeslot < 4*num_frames; timeslot++) {
                    vector<char> default_val(w * packet_size, char(0));
                    auto n_cols = uint16_t(n_data_p_f * num_frames);
                    auto n_rows = uint16_t(n_par_p_f * num_frames);
                    vector<vector<char>> data;
                    for (uint16_t j = 0; j < n_cols; j++) {
                      vector<char> data_row;
                      for (uint16_t l = 0; l < w * packet_size; l++) {
                        data_row.push_back(char(3*j+1));
                      }
                      data.push_back(data_row);
                    }
                    CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
                    BlockCode encodeBlockCode(codingMatrixInfo, delay,
                        { 1 , u }, packet_size, bitmatrix > 0);
                    BlockCode decodeBlockCode(codingMatrixInfo, delay,
                        { 1 , u }, packet_size, bitmatrix > 0);
                    for (uint16_t time = 0; time < timeslot; time++) {
                      encodeBlockCode.update_timeslot(time);
                      decodeBlockCode.update_timeslot(time);
                      auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
                      for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                        encodeBlockCode.place_payload(pos, false, data.at(pos));
                        decodeBlockCode.place_payload(pos, false, data.at(pos));
                      }
                    }
                    encodeBlockCode.update_timeslot(timeslot);
                    decodeBlockCode.update_timeslot(timeslot);

                    auto endpoints = cols_of_frame(timeslot, delay, codingMatrixInfo);
                    auto missing = get_missing(n_data_p_f, rand() % (min(n_data_p_f,
                        n_par_p_f) + 1));
                    uint16_t num_missing = 0;
                    for (uint16_t pos = endpoints.first; pos <= endpoints.second;
                        pos++) {
                      if (!missing.at(pos -endpoints.first)) {
                        decodeBlockCode.place_payload(pos, false, data.at(pos));
                      }
                      else { num_missing++; }
                      encodeBlockCode.place_payload(pos, false, data.at(pos));
                    }
                    encodeBlockCode.encode();
                    auto par_endpoints = rows_of_frame(timeslot, delay,
                        codingMatrixInfo);
                    auto missing_par = get_missing(n_par_p_f, rand() % (n_par_p_f -
                        num_missing + 1));
                    for (uint16_t pos = par_endpoints.first; pos <=
                        par_endpoints.second; pos++) {
                      if (!missing_par.at(pos - par_endpoints.first)) {
                        const auto val = encodeBlockCode.get_row(pos, true);
                        vector<char> payload;
                        for (uint16_t j = 0; j < val.size(); j++) {
                          payload.push_back(val.at(j));
                        }
                        decodeBlockCode.place_payload(pos, true, payload);
                      }
                    }
                    if (num_missing == 0) {
                      EXPECT_FALSE(decodeBlockCode.can_recover());
                      continue;
                    }
                    else {
                      EXPECT_TRUE(decodeBlockCode.can_recover());
                    }
                    auto decoded = decodeBlockCode.decode();
                    uint16_t num_decoded = 0;
                    for (auto el : decoded) { num_decoded += el; }
                    for (uint16_t pos = endpoints.first; pos <= endpoints.second;
                        pos++) {
                      EXPECT_EQ(decodeBlockCode.get_row(pos, false).size(),
                          packet_size * w);
                      for (uint16_t i = 0; i < packet_size * w; i++) {
                        EXPECT_EQ(decodeBlockCode.get_row(pos, false).at(i),
                          data.at(pos).at(i));
                      }
                      EXPECT_FALSE(decodeBlockCode.get_erased().at(pos));
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
}

TEST(test_block_code, test_get_recoverable_data_burst_full_recover)
{
  for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
    for (uint16_t w = 8; w < 33; w += 8) {
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 1; n_data_p_f < 5; n_data_p_f++) {
        uint16_t n_par_p_f = n_data_p_f;
        uint16_t delay = 3;
        auto num_frames = num_frames_for_delay(delay);
        for (uint16_t exp_loss = 0; exp_loss < 4; exp_loss++) {
          for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
            for (uint16_t timeslot = num_frames - 1; timeslot < 4*num_frames; timeslot++) {
              auto shift = num_frames - 1;
              vector<char> default_val(w * packet_size, char(0));
              auto n_cols = uint16_t(n_data_p_f * num_frames);
              auto n_rows = uint16_t(n_par_p_f * num_frames);
              vector<vector<char>> data;
              for (uint16_t j = 0; j < n_cols; j++) {
                vector<char> data_row;
                for (uint16_t l = 0; l < w * packet_size; l++) {
                  data_row.push_back(char(3*j+1));
                }
                data.push_back(data_row);
              }
              CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
              BlockCode encodeBlockCode(codingMatrixInfo, delay,
                  { 1 , 0 }, packet_size, bitmatrix > 0);
              BlockCode decodeBlockCode(codingMatrixInfo, delay,
                  { 1 , 0 }, packet_size, bitmatrix > 0);
              for (uint16_t time = 0; time <= timeslot; time++) {
                encodeBlockCode.update_timeslot(time);
                decodeBlockCode.update_timeslot(time);
                auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
                bool skip_0 = (time % num_frames) == ((timeslot - shift) %
                    num_frames )and (exp_loss % 2);
                bool skip_1 = (time % num_frames) == ((timeslot - shift + 1)
                    % num_frames) and (exp_loss / 2);
                for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                  encodeBlockCode.place_payload(pos, false, data.at(pos));
                  if (time % num_frames != ((timeslot - shift + 3) % num_frames)
                      and time % num_frames != ((timeslot - shift + 4)
                      % num_frames)) {
                    if (!skip_0 and !skip_1) {
                      decodeBlockCode.place_payload(pos, false, data.at(pos));
                    }
                  }
                  encodeBlockCode.encode();
                }
                auto par_endpoints = rows_of_frame(time, delay,
                    codingMatrixInfo);
                for (uint16_t pos = par_endpoints.first; pos <=
                    par_endpoints.second; pos++) {
                  const auto val = encodeBlockCode.get_row(pos, true);
                  vector<char> payload;
                  for (uint16_t j = 0; j < val.size(); j++) {
                    payload.push_back(val.at(j));
                  }
                  if (!skip_0 and !skip_1 and (time % num_frames != ((timeslot -
                      shift + 3) % num_frames) and time % num_frames != (timeslot
                      - shift + 4) % num_frames)) {
                    decodeBlockCode.place_payload(pos, true, payload);
                  }
                }
              }
              EXPECT_TRUE(decodeBlockCode.can_recover());
              auto decoded = decodeBlockCode.decode();
              uint16_t num_decoded = 0;
              for (auto el : decoded) { num_decoded += el; }
              for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
                EXPECT_EQ(decodeBlockCode.get_row(pos, false).size(),
                    packet_size * w);
                auto frame = frame_of_col(pos, delay, codingMatrixInfo);
                bool lost = ((frame % num_frames) == ((timeslot - shift) %
                    num_frames) and (exp_loss % 2)) or ((frame % num_frames) == (
                    (timeslot - shift + 1) % num_frames) and (exp_loss / 2));
                for (uint16_t i = 0; i < packet_size * w; i++) {
                  if (!lost) {
                    EXPECT_EQ(decodeBlockCode.get_row(pos, false).at(i),
                        data.at(pos).at(i));
                    EXPECT_FALSE(decodeBlockCode.get_erased().at(pos));
                  }
                  else {
                    EXPECT_TRUE(decodeBlockCode.get_erased().at(pos));
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

TEST(test_block_code, test_get_recoverable_data_burst_partial_recover)
{
  for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
    for (uint16_t w = 8; w < 33; w += 8) {
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 2; n_data_p_f < 10; n_data_p_f+= 2) {
        for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
          uint16_t n_par_p_f = n_data_p_f;
          uint16_t delay = 3;
          auto num_frames = num_frames_for_delay(delay);
          for (uint16_t timeslot = num_frames - 1; timeslot < 4*num_frames; timeslot++) {
            auto shift = num_frames - 1;
            vector<char> default_val(w * packet_size, char(0));
            auto n_cols = uint16_t(n_data_p_f * num_frames);
            auto n_rows = uint16_t(n_par_p_f * num_frames);
            vector<vector<char>> data;
            for (uint16_t j = 0; j < n_cols; j++) {
              vector<char> data_row;
              for (uint16_t l = 0; l < w * packet_size; l++) {
                data_row.push_back(char(3*j+1));
              }
              data.push_back(data_row);
            }
            CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
            BlockCode encodeBlockCode(codingMatrixInfo, delay,
                { 1 , 1 }, packet_size, bitmatrix > 0);
            BlockCode decodeBlockCode(codingMatrixInfo, delay,
                { 1 , 1 }, packet_size, bitmatrix > 0);
            for (uint16_t time = 0; time <= timeslot; time++) {
              encodeBlockCode.update_timeslot(time);
              decodeBlockCode.update_timeslot(time);
              auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
              for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                encodeBlockCode.place_payload(pos, false, data.at(pos));
                if (time % num_frames != ((timeslot - shift + 3) % num_frames)
                    and time % num_frames != ((timeslot - shift + 4)
                    % num_frames)) {
                  decodeBlockCode.place_payload(pos, false, data.at(pos));
                }
                encodeBlockCode.encode();
              }
              auto par_endpoints = rows_of_frame(time, delay,
                  codingMatrixInfo);
              for (uint16_t pos = par_endpoints.first; pos <=
                  par_endpoints.second; pos++) {
                if (time % num_frames != ((timeslot - shift + 3) % num_frames)
                    and time % num_frames != ((timeslot - shift + 4)
                    % num_frames)) {
                  const auto val = encodeBlockCode.get_row(pos, true);
                  vector<char> payload;
                  for (uint16_t j = 0; j < val.size(); j++) {
                    payload.push_back(val.at(j));
                  }
                  decodeBlockCode.place_payload(pos, true, payload);
                }
              }
            }
            StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
            EXPECT_TRUE(decodeBlockCode.can_recover());
            auto decoded = decodeBlockCode.decode();
            uint16_t num_decoded = 0;
            for (auto el : decoded) { num_decoded += el; }
            for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
              EXPECT_EQ(decodeBlockCode.get_row(pos, false).size(),
                  packet_size * w);
              auto frame = frame_of_col(pos, delay, codingMatrixInfo);
              for (uint16_t i = 0; i < packet_size * w; i++) {
                bool u_lost = frame == ((timeslot - shift + 4) % num_frames)
                    and helper.get_positions_of_us().at(pos);
                if (!u_lost) {
                  EXPECT_EQ(decodeBlockCode.get_row(pos, false).at(i),
                    data.at(pos).at(i));
                  EXPECT_FALSE(decodeBlockCode.get_erased().at(pos));
                }
                else {
                  EXPECT_TRUE(decodeBlockCode.get_erased().at(pos));
                }
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_block_code, test_get_recoverable_data_burst_partial_recover_exp_plus)
{
  for (uint16_t packet_size = 8; packet_size < 24; packet_size+= 8) {
    for (uint16_t w = 8; w < 33; w += 8) {
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 2; n_data_p_f < 10; n_data_p_f+= 2) {
        for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
          uint16_t n_par_p_f = n_data_p_f;
          uint16_t delay = 3;
          auto num_frames = num_frames_for_delay(delay);
          for (uint16_t timeslot = num_frames - 1; timeslot < 4*num_frames; timeslot++) {
            auto shift = num_frames - 1;
            vector<char> default_val(w * packet_size, char(0));
            auto n_cols = uint16_t(n_data_p_f * num_frames);
            auto n_rows = uint16_t(n_par_p_f * num_frames);
            vector<vector<char>> data;
            for (uint16_t j = 0; j < n_cols; j++) {
              vector<char> data_row;
              for (uint16_t l = 0; l < w * packet_size; l++) {
                data_row.push_back(char(3*j+1));
              }
              data.push_back(data_row);
            }
            CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
            BlockCode encodeBlockCode(codingMatrixInfo, delay,
                { 1 , 1 }, packet_size, bitmatrix > 0);
            BlockCode decodeBlockCode(codingMatrixInfo, delay,
                { 1 , 1 }, packet_size, bitmatrix > 0);
            for (uint16_t time = 0; time <= timeslot; time++) {
              encodeBlockCode.update_timeslot(time);
              decodeBlockCode.update_timeslot(time);
              auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
              for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                encodeBlockCode.place_payload(pos, false, data.at(pos));
                if (time % num_frames != ((timeslot - shift + 3) % num_frames)
                    and (time % num_frames) != ((timeslot - shift ) % num_frames)
                    and (time % num_frames) != ((timeslot - shift + 1) % num_frames)) {
                  decodeBlockCode.place_payload(pos, false, data.at(pos));
                }
                encodeBlockCode.encode();
              }
              auto par_endpoints = rows_of_frame(time, delay,
                  codingMatrixInfo);
              for (uint16_t pos = par_endpoints.first; pos <=
                  par_endpoints.second; pos++) {
                if (time % num_frames != ((timeslot - shift + 3) % num_frames)
                    and (time % num_frames) != ((timeslot - shift ) % num_frames)
                    and (time % num_frames) != ((timeslot - shift + 1) % num_frames)) {
                  const auto val = encodeBlockCode.get_row(pos, true);
                  vector<char> payload;
                  for (uint16_t j = 0; j < val.size(); j++) {
                    payload.push_back(val.at(j));
                  }
                  decodeBlockCode.place_payload(pos, true, payload);
                }
              }
            }
            StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
            EXPECT_TRUE(decodeBlockCode.can_recover());
            auto decoded = decodeBlockCode.decode();
            uint16_t num_decoded = 0;
            for (auto el : decoded) { num_decoded += el; }
            for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
              EXPECT_EQ(decodeBlockCode.get_row(pos, false).size(),
                  packet_size * w);
              auto frame = frame_of_col(pos, delay, codingMatrixInfo);
              for (uint16_t i = 0; i < packet_size * w; i++) {
                bool lost = frame == ((timeslot - shift + 0) % num_frames);
                if (!lost) {
                  EXPECT_EQ(decodeBlockCode.get_row(pos, false).at(i),
                    data.at(pos).at(i));
                  EXPECT_FALSE(decodeBlockCode.get_erased().at(pos));
                }
                else {
                  EXPECT_TRUE(decodeBlockCode.get_erased().at(pos));
                }
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_block_code, test_get_recoverable_data_burst_full_recover_exp_plus)
{
  for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
    for (uint16_t w = 8; w < 33; w += 8) {
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 2; n_data_p_f < 10; n_data_p_f+= 2) {
        for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
          uint16_t n_par_p_f = n_data_p_f * 2;
          uint16_t delay = 3;
          auto num_frames = num_frames_for_delay(delay);
          for (uint16_t timeslot = num_frames - 1; timeslot < 4*num_frames; timeslot++) {
            auto shift = num_frames - 1;
            vector<char> default_val(w * packet_size, char(0));
            auto n_cols = uint16_t(n_data_p_f * num_frames);
            auto n_rows = uint16_t(n_par_p_f * num_frames);
            vector<vector<char>> data;
            for (uint16_t j = 0; j < n_cols; j++) {
              vector<char> data_row;
              for (uint16_t l = 0; l < w * packet_size; l++) {
                data_row.push_back(char(3*j+1));
              }
              data.push_back(data_row);
            }
            CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
            BlockCode encodeBlockCode(codingMatrixInfo, delay,
                { 1 , 1 }, packet_size, bitmatrix > 0);
            BlockCode decodeBlockCode(codingMatrixInfo, delay,
                { 1 , 1 }, packet_size, bitmatrix > 0);
            for (uint16_t time = 0; time <= timeslot; time++) {
              encodeBlockCode.update_timeslot(time);
              decodeBlockCode.update_timeslot(time);
              auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
              for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                encodeBlockCode.place_payload(pos, false, data.at(pos));
                if (time % num_frames != ((timeslot - shift + 2) % num_frames)
                    and (time % num_frames) != ((timeslot - shift + 3) %
                    num_frames) and (time % num_frames) != ((timeslot - shift +
                    4) % num_frames)) {
                  decodeBlockCode.place_payload(pos, false, data.at(pos));
                }
                encodeBlockCode.encode();
              }
              auto par_endpoints = rows_of_frame(time, delay,
                  codingMatrixInfo);
              for (uint16_t pos = par_endpoints.first; pos <=
                  par_endpoints.second; pos++) {
                if (time % num_frames != ((timeslot - shift + 2) % num_frames)
                    and (time % num_frames) != ((timeslot - shift + 3) %
                    num_frames) and (time % num_frames) != ((timeslot - shift +
                    4) % num_frames)) {
                  const auto val = encodeBlockCode.get_row(pos, true);
                  vector<char> payload;
                  for (uint16_t j = 0; j < val.size(); j++) {
                    payload.push_back(val.at(j));
                  }
                  decodeBlockCode.place_payload(pos, true, payload);
                }
              }
            }
            StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
            EXPECT_TRUE(decodeBlockCode.can_recover());
            auto decoded = decodeBlockCode.decode();
            uint16_t num_decoded = 0;
            for (auto el : decoded) { num_decoded += el; }
            for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
              EXPECT_EQ(decodeBlockCode.get_row(pos, false).size(),
                  packet_size * w);
              auto frame = frame_of_col(pos, delay, codingMatrixInfo);
              for (uint16_t i = 0; i < packet_size * w; i++) {
                bool u_lost = frame == ((timeslot - shift + 4) % num_frames)
                    and helper.get_positions_of_us().at(pos);
                if (!u_lost) {
                  EXPECT_EQ(decodeBlockCode.get_row(pos, false).at(i),
                    data.at(pos).at(i));
                  EXPECT_FALSE(decodeBlockCode.get_erased().at(pos));
                }
                else {
                  EXPECT_TRUE(decodeBlockCode.get_erased().at(pos));
                }
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_block_code, test_get_recoverable_data_burst_v_recover)
{
  for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
    for (uint16_t w = 8; w < 33; w += 8) {
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 2; n_data_p_f < 9; n_data_p_f+= 2) {
        for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
          uint16_t n_par_p_f = n_data_p_f;
          uint16_t delay = 3;
          auto num_frames = num_frames_for_delay(delay);
          for (uint16_t timeslot = num_frames - 1; timeslot < 4*num_frames; timeslot++) {
            auto shift = num_frames - 1;
            vector<char> default_val(w * packet_size, char(0));
            auto n_cols = uint16_t(n_data_p_f * num_frames);
            auto n_rows = uint16_t(n_par_p_f * num_frames);
            vector<vector<char>> data;
            for (uint16_t j = 0; j < n_cols; j++) {
              vector<char> data_row;
              for (uint16_t l = 0; l < w * packet_size; l++) {
                data_row.push_back(char(3*j+1));
              }
              data.push_back(data_row);
            }
            CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
            BlockCode encodeBlockCode(codingMatrixInfo, delay,
                { 1 , 1 }, packet_size, bitmatrix > 0);
            BlockCode decodeBlockCode(codingMatrixInfo, delay,
                { 1 , 1 }, packet_size, bitmatrix > 0);
            for (uint16_t time = 0; time <= timeslot; time++) {
              encodeBlockCode.update_timeslot(time);
              decodeBlockCode.update_timeslot(time);
              auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
              for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                encodeBlockCode.place_payload(pos, false, data.at(pos));
                if (time % num_frames != ((timeslot - shift + 4) % num_frames)
                    and time % num_frames != ((timeslot - shift + 5)
                    % num_frames)) {
                  decodeBlockCode.place_payload(pos, false, data.at(pos));
                }
                encodeBlockCode.encode();
              }
              auto par_endpoints = rows_of_frame(time, delay,
                  codingMatrixInfo);
              for (uint16_t pos = par_endpoints.first; pos <=
                  par_endpoints.second; pos++) {
                if (time % num_frames != ((timeslot - shift + 4) % num_frames)
                    and time % num_frames != ((timeslot - shift + 5)
                    % num_frames)) {
                  const auto val = encodeBlockCode.get_row(pos, true);
                  vector<char> payload;
                  for (uint16_t j = 0; j < val.size(); j++) {
                    payload.push_back(val.at(j));
                  }
                  decodeBlockCode.place_payload(pos, true, payload);
                }
              }
            }
            StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
            EXPECT_TRUE(decodeBlockCode.can_recover());
            auto decoded = decodeBlockCode.decode();
            uint16_t num_decoded = 0;
            for (auto el : decoded) { num_decoded += el; }
            for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
              EXPECT_EQ(decodeBlockCode.get_row(pos, false).size(),
                  packet_size * w);
              auto frame = frame_of_col(pos, delay, codingMatrixInfo);
              for (uint16_t i = 0; i < packet_size * w; i++) {
                bool u_lost = ((frame == ((timeslot - shift + 4) % num_frames))
                    or (frame == ((timeslot - shift + 5) % num_frames)))
                    and helper.get_positions_of_us().at(pos);
                if (!u_lost) {
                  EXPECT_EQ(decodeBlockCode.get_row(pos, false).at(i),
                    data.at(pos).at(i));
                  EXPECT_FALSE(decodeBlockCode.get_erased().at(pos));
                }
                else {
                  EXPECT_TRUE(decodeBlockCode.get_erased().at(pos));
                }
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_block_code, test_get_recoverable_data_burst_not_recovered)
{
  for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
    for (uint16_t w = 8; w < 33; w += 8) {
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 2; n_data_p_f < 9; n_data_p_f+= 2) {
        for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
          uint16_t n_par_p_f = n_data_p_f / 2;
          uint16_t delay = 3;
          auto num_frames = num_frames_for_delay(delay);
          for (uint16_t timeslot = num_frames - 1; timeslot < 4*num_frames; timeslot++) {
            auto shift = num_frames - 1;
            vector<char> default_val(w * packet_size, char(0));
            auto n_cols = uint16_t(n_data_p_f * num_frames);
            auto n_rows = uint16_t(n_par_p_f * num_frames);
            vector<vector<char>> data;
            for (uint16_t j = 0; j < n_cols; j++) {
              vector<char> data_row;
              for (uint16_t l = 0; l < w * packet_size; l++) {
                data_row.push_back(char(3*j+1));
              }
              data.push_back(data_row);
            }
            CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
            BlockCode encodeBlockCode(codingMatrixInfo, delay,
                { 1 , 1 }, packet_size, bitmatrix > 0);
            BlockCode decodeBlockCode(codingMatrixInfo, delay,
                { 1 , 1 }, packet_size, bitmatrix > 0);
            for (uint16_t time = 0; time <= timeslot; time++) {
              encodeBlockCode.update_timeslot(time);
              decodeBlockCode.update_timeslot(time);
              auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
              for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                encodeBlockCode.place_payload(pos, false, data.at(pos));
                if (time % num_frames != ((timeslot - shift + 3) % num_frames)
                    and time % num_frames != ((timeslot - shift + 4)
                    % num_frames)) {
                  decodeBlockCode.place_payload(pos, false, data.at(pos));
                }
                encodeBlockCode.encode();
              }
              auto par_endpoints = rows_of_frame(time, delay,
                  codingMatrixInfo);
              for (uint16_t pos = par_endpoints.first; pos <=
                  par_endpoints.second; pos++) {
                if (time % num_frames != ((timeslot - shift + 3) % num_frames)
                    and time % num_frames != ((timeslot - shift + 4)
                    % num_frames)) {
                  const auto val = encodeBlockCode.get_row(pos, true);
                  vector<char> payload;
                  for (uint16_t j = 0; j < val.size(); j++) {
                    payload.push_back(val.at(j));
                  }
                  decodeBlockCode.place_payload(pos, true, payload);
                }
              }
            }
            StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
            EXPECT_FALSE(decodeBlockCode.can_recover());
            auto decoded = decodeBlockCode.decode();
            uint16_t num_decoded = 0;
            for (auto el : decoded) { num_decoded += el; }
            for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
              EXPECT_EQ(decodeBlockCode.get_row(pos, false).size(),
                  packet_size * w);
              auto frame = frame_of_col(pos, delay, codingMatrixInfo);
              for (uint16_t i = 0; i < packet_size * w; i++) {
                bool lost = ((frame == ((timeslot - shift + 3) % num_frames))
                    or (frame == ((timeslot - shift + 4) % num_frames)));
                if (!lost) {
                  EXPECT_EQ(decodeBlockCode.get_row(pos, false).at(i),
                      data.at(pos).at(i));
                  EXPECT_FALSE(decodeBlockCode.get_erased().at(pos));
                }
                else {
                  EXPECT_TRUE(decodeBlockCode.get_erased().at(pos));
                }
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_block_code, test_get_recoverable_data_burst_full_recover_exp)
{
  for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
    for (uint16_t w = 8; w < 33; w += 8) {
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 1; n_data_p_f < 5; n_data_p_f++) {
        uint16_t n_par_p_f = n_data_p_f;
        uint16_t delay = 3;
        auto num_frames = num_frames_for_delay(delay);
        for (uint16_t shift_start = 3; shift_start > 0; shift_start--) {
          for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
            for (uint16_t timeslot = num_frames - 1; timeslot < 4*num_frames; timeslot++) {
              auto shift = num_frames - 1;
              vector<char> default_val(w * packet_size, char(0));
              auto n_cols = uint16_t(n_data_p_f * num_frames);
              auto n_rows = uint16_t(n_par_p_f * num_frames);
              vector<vector<char>> data;
              for (uint16_t j = 0; j < n_cols; j++) {
                vector<char> data_row;
                for (uint16_t l = 0; l < w * packet_size; l++) {
                  data_row.push_back(char(3*j+1));
                }
                data.push_back(data_row);
              }
              CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
              BlockCode encodeBlockCode(codingMatrixInfo, delay,
                  { 1 , 0 }, packet_size, bitmatrix > 0);
              BlockCode decodeBlockCode(codingMatrixInfo, delay,
                  { 1 , 0 }, packet_size, bitmatrix > 0);
              for (uint16_t time = 0; time <= timeslot; time++) {
                encodeBlockCode.update_timeslot(time);
                decodeBlockCode.update_timeslot(time);
                auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
                for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                  encodeBlockCode.place_payload(pos, false, data.at(pos));
                  if (time % num_frames != ((timeslot - shift + 3 - shift_start)
                      % num_frames) and time % num_frames != ((timeslot - shift +
                      4 - shift_start) % num_frames)) {
                    decodeBlockCode.place_payload(pos, false, data.at(pos));
                  }
                  encodeBlockCode.encode();
                }
                auto par_endpoints = rows_of_frame(time, delay,
                    codingMatrixInfo);
                for (uint16_t pos = par_endpoints.first; pos <=
                    par_endpoints.second; pos++) {
                  if (time % num_frames != ((timeslot - shift + 3 - shift_start)
                      % num_frames) and time % num_frames != ((timeslot - shift +
                      4 - shift_start) % num_frames)) {
                    const auto val = encodeBlockCode.get_row(pos, true);
                    vector<char> payload;
                    for (uint16_t j = 0; j < val.size(); j++) {
                      payload.push_back(val.at(j));
                    }
                    decodeBlockCode.place_payload(pos, true, payload);
                  }
                }
              }
              EXPECT_TRUE(decodeBlockCode.can_recover());
              auto decoded = decodeBlockCode.decode();
              uint16_t num_decoded = 0;
              for (auto el : decoded) { num_decoded += el; }
              for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
                EXPECT_EQ(decodeBlockCode.get_row(pos, false).size(),
                    packet_size * w);
                for (uint16_t i = 0; i < packet_size * w; i++) {
                  EXPECT_EQ(decodeBlockCode.get_row(pos, false).at(i),
                    data.at(pos).at(i));
                }
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_block_code, test_get_recoverable_data_burst_not_recovered_exp)
{
  for (uint16_t packet_size = 8; packet_size < 25; packet_size+= 8) {
    for (uint16_t w = 8; w < 33; w += 8) {
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 2; n_data_p_f < 9; n_data_p_f+= 2) {
        uint16_t n_par_p_f = n_data_p_f / 2;
        uint16_t delay = 3;
        auto num_frames = num_frames_for_delay(delay);
        for (uint16_t shift_start = 3; shift_start > 0; shift_start--) {
          for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
            for (uint16_t timeslot = num_frames - 1; timeslot < 4*num_frames; timeslot++) {
              auto shift = num_frames - 1;
              vector<char> default_val(w * packet_size, char(0));
              auto n_cols = uint16_t(n_data_p_f * num_frames);
              auto n_rows = uint16_t(n_par_p_f * num_frames);
              vector<vector<char>> data;
              for (uint16_t j = 0; j < n_cols; j++) {
                vector<char> data_row;
                for (uint16_t l = 0; l < w * packet_size; l++) {
                  data_row.push_back(char(3*j+1));
                }
                data.push_back(data_row);
              }
              CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
              BlockCode encodeBlockCode(codingMatrixInfo, delay,
                  { 1 , 1 }, packet_size, bitmatrix > 0);
              BlockCode decodeBlockCode(codingMatrixInfo, delay,
                  { 1 , 1 }, packet_size, bitmatrix > 0);
              for (uint16_t time = 0; time <= timeslot; time++) {
                encodeBlockCode.update_timeslot(time);
                decodeBlockCode.update_timeslot(time);
                auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
                for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                  encodeBlockCode.place_payload(pos, false, data.at(pos));
                  if (time % num_frames != ((timeslot - shift + 3 - shift_start)
                      % num_frames) and time % num_frames != ((timeslot - shift +
                      4 - shift_start) % num_frames)) {
                    decodeBlockCode.place_payload(pos, false, data.at(pos));
                  }
                  encodeBlockCode.encode();
                }
                auto par_endpoints = rows_of_frame(time, delay,
                    codingMatrixInfo);
                for (uint16_t pos = par_endpoints.first; pos <=
                    par_endpoints.second; pos++) {
                  if (time % num_frames != ((timeslot - shift + 3 - shift_start)
                      % num_frames) and time % num_frames != ((timeslot - shift +
                      4 - shift_start) % num_frames)) {
                    const auto val = encodeBlockCode.get_row(pos, true);
                    vector<char> payload;
                    for (uint16_t j = 0; j < val.size(); j++) {
                      payload.push_back(val.at(j));
                    }
                    decodeBlockCode.place_payload(pos, true, payload);
                  }
                }
              }
              StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
              EXPECT_FALSE(decodeBlockCode.can_recover());
              auto decoded = decodeBlockCode.decode();
              uint16_t num_decoded = 0;
              for (auto el : decoded) { num_decoded += el; }
              for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
                EXPECT_EQ(decodeBlockCode.get_row(pos, false).size(),
                    packet_size * w);
                auto frame = frame_of_col(pos, delay, codingMatrixInfo);
                for (uint16_t i = 0; i < packet_size * w; i++) {
                  bool lost = ((frame == ((timeslot - shift + 3 - shift_start) %
                      num_frames)) or (frame == ((timeslot - shift + 4 -
                      shift_start) % num_frames)));
                  EXPECT_EQ(decodeBlockCode.get_row(pos, false).at(i),
                    lost? default_val.at(i) : data.at(pos).at(i));
                }
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_block_code, test_encode_time)
{
  srand(0);
  for (uint16_t n_par_p_f = 9; n_par_p_f < 10; n_par_p_f++) {
    for (uint16_t n_data_p_f = 19; n_data_p_f < 20; n_data_p_f++) {
      for (uint16_t delay = 0; delay < 4; delay++) {
        for (uint16_t w = 8; w < 33; w += 8) {
          for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
            if (w == 24) { continue; }
            uint16_t packet_size = 8 * (1000 / w);
            auto num_frames = num_frames_for_delay(delay);
            float total_time = 0;
            float count = 0;
            for (uint16_t timeslot = 0; timeslot < 100; timeslot++) {
              vector<char> default_val(w * packet_size, char(0));
              auto n_cols = uint16_t(n_data_p_f * num_frames);
              auto n_rows = uint16_t(n_par_p_f * num_frames);
              vector<vector<char>> data;
              for (uint16_t j = 0; j < n_cols + n_rows; j++) {
                vector<char> data_row;
                for (uint16_t l = 0; l < w * packet_size; l++) {
                  data_row.push_back(char(j+1));
                }
                data.push_back(data_row);
              }
              CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
              BlockCode encodeBlockCode(codingMatrixInfo, delay,
                  { 1 , 1 }, packet_size, bitmatrix > 0);
              for (uint16_t pos = 0; pos < n_cols; pos++) {
                encodeBlockCode.place_payload(pos, false, data.at(pos));
              }
              for (uint16_t pos = 0; pos < n_rows; pos++) {
                encodeBlockCode.place_payload(pos, true, data.at(pos + n_cols));
              }
              for (uint16_t time = 0; time < timeslot; time++) {
                encodeBlockCode.update_timeslot(time);
                auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
                for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                  encodeBlockCode.place_payload(pos, false, data.at(pos));
                }
                auto rows_frame = rows_of_frame(time, delay, codingMatrixInfo);
                for (uint16_t pos = rows_frame.first; pos <= rows_frame.second; pos++) {
                  encodeBlockCode.place_payload(pos, true, data.at(n_cols + pos));
                }
              }
              encodeBlockCode.update_timeslot(timeslot);
              auto endpoints = cols_of_frame(timeslot, delay, codingMatrixInfo);
              for (uint16_t pos = endpoints.first; pos <= endpoints.second;
                  pos++) {
                encodeBlockCode.place_payload(pos, false, data.at(pos));
              }
              auto start = clock();
              encodeBlockCode.encode();
              auto end = clock();
              auto time = 1000*float(end-start) / CLOCKS_PER_SEC;
              total_time += time;
              count++;
            }
            EXPECT_LE(total_time / count, bitmatrix? 30.0 : 2.0);
          }
        }
      }
    }
  }
}

TEST(test_block_code, test_decode_time)
{
  srand(0);
  for (uint16_t n_par_p_f = 9; n_par_p_f < 10; n_par_p_f++) {
    for (uint16_t n_data_p_f = 19; n_data_p_f < 20; n_data_p_f++) {
      for (uint16_t delay = 3; delay < 4; delay++) {
        for (uint16_t w = 8; w < 33; w += 8) {
          for (uint16_t bitmatrix = 0; bitmatrix < 2; bitmatrix++) {
            if (w == 24) { continue; }
            uint16_t packet_size = 8 * (1000 / w);
            auto num_frames = num_frames_for_delay(delay);
            float total_time = 0;
            float count = 0;
            for (uint16_t timeslot = num_frames - 1; timeslot <  num_frames - 1 +
                100; timeslot++) {
              auto shift = num_frames - 1;
              vector<char> default_val(w * packet_size, char(0));
              auto n_cols = uint16_t(n_data_p_f * num_frames);
              auto n_rows = uint16_t(n_par_p_f * num_frames);
              vector<vector<char>> data;
              for (uint16_t j = 0; j < n_cols; j++) {
                vector<char> data_row;
                for (uint16_t l = 0; l < w * packet_size; l++) {
                  data_row.push_back(char(3*j+1));
                }
                data.push_back(data_row);
              }
              CodingMatrixInfo codingMatrixInfo { n_rows, n_cols, w };
              BlockCode encodeBlockCode(codingMatrixInfo, delay,
                  { 1 , 0 }, packet_size, bitmatrix > 0);
              BlockCode decodeBlockCode(codingMatrixInfo, delay,
                  { 1 , 0 }, packet_size, bitmatrix > 0);
              for (uint16_t time = 0; time <= timeslot; time++) {
                encodeBlockCode.update_timeslot(time);
                decodeBlockCode.update_timeslot(time);
                auto cols_frame = cols_of_frame(time, delay, codingMatrixInfo);
                for (uint16_t pos = cols_frame.first; pos <= cols_frame.second; pos++) {
                  encodeBlockCode.place_payload(pos, false, data.at(pos));
                  if (time % num_frames != ((timeslot - shift + 3) % num_frames)
                      and time % num_frames != ((timeslot - shift + 4)
                      % num_frames)) {
                    decodeBlockCode.place_payload(pos, false, data.at(pos));
                  }
                  encodeBlockCode.encode();
                }
                auto par_endpoints = rows_of_frame(time, delay,
                    codingMatrixInfo);
                for (uint16_t pos = par_endpoints.first; pos <=
                    par_endpoints.second; pos++) {
                  const auto val = encodeBlockCode.get_row(pos, true);
                  vector<char> payload;
                  for (uint16_t j = 0; j < val.size(); j++) {
                    payload.push_back(val.at(j));
                  }
                  if ((time % num_frames != ((timeslot - shift + 3) % num_frames)
                      and time % num_frames != (timeslot - shift + 4) %
                      num_frames)) {
                    decodeBlockCode.place_payload(pos, true, payload);
                  }
                }
              }
              auto start = clock();
              auto decoded = decodeBlockCode.decode();
              auto end = clock();
              auto time = 1000*float(end-start) / CLOCKS_PER_SEC;
              total_time += time;
              count++;
            }
            EXPECT_LE(total_time / count, 2.0);
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
