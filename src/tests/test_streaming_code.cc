#include "gtest/gtest.h"
#include <chrono>
#include <iostream>
#include <cmath>
#include <random>
#include <thread>
#include "streaming_code.hh"
#include "helpers.hh"

using namespace std;

TEST(test_streaming_code,
    test_updated_ts)
{
  srand(0);
  for (uint16_t tau = 0; tau < 4; tau++) {
    for (uint16_t trial = 0; trial < 20; trial++) {
      for (uint16_t size_factor = 1; size_factor < 2; size_factor++) {
        uint16_t w = 32;
        uint16_t max_frame_size_possible = 10000;
        uint16_t packet_size = 8 * size_factor;
        uint16_t stripe_size = packet_size * w;
        uint16_t max_data_frame_pkts = 100;
        uint16_t max_fec_frame_pkts = 50;
        uint16_t max_data_stripes_per_frame = 50;
        uint16_t max_fec_stripes_per_frame = 25;
        assert (max_data_stripes_per_frame * stripe_size >= max_frame_size_possible);
        assert (max_fec_stripes_per_frame * stripe_size >= max_frame_size_possible / 2);
        assert (pow(2, w) >= uint64_t(num_frames_for_delay(tau)) * uint64_t(num_frames_for_delay(tau))
            * max_data_stripes_per_frame * max_fec_stripes_per_frame);
        CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(tau) *
            max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * max_data_stripes_per_frame), w};

        BlockCode blockCodeFEC(codingMatrixInfoFEC, tau,
            pair<uint16_t, uint16_t>{1, 1 }, packet_size, false);
        StreamingCode code(tau, stripe_size,  &blockCodeFEC, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);
        uint16_t ts = 0;
        uint16_t num_frames = num_frames_for_delay(tau);
        uint16_t num_init = rand() % (num_frames * 5);
        while (ts < num_init) {
          EXPECT_EQ(ts,code.updated_ts(ts));
          code.set_timeslot(ts);
          ts++;
        }
        uint16_t extra = (rand() % (2 * num_frames));
        ts += extra;
        for (uint16_t s = 0; s < max_data_stripes_per_frame * num_frames; s++) {
          blockCodeFEC.place_payload(s, false, vector<char>(1,char(1)));
        }
        EXPECT_EQ(ts, code.updated_ts(ts));
        uint16_t start = num_init;
        uint16_t end = ts;
        uint16_t end_check = start + num_frames;
        if (start + num_frames - 1 <= ts) {
          end = start + num_frames - 1;
          end_check = end - 1;
        }
        if (tau == 0) {
          start = ts;
          end = ts;
          end_check = 0;
        }
        for (uint16_t j = start; j <= end; j++) {
          auto row = first_pos_of_frame_in_block_code(j, num_frames,
              max_data_stripes_per_frame, max_fec_stripes_per_frame, false);
          EXPECT_EQ(char(0), blockCodeFEC.get_row(row, false).at(0));
        }
        for (uint16_t j = end + 1; j < end_check; j++) {
          auto row = first_pos_of_frame_in_block_code(j, num_frames,
              max_data_stripes_per_frame, max_fec_stripes_per_frame, false);
          EXPECT_EQ(char(1), blockCodeFEC.get_row(row, false).at(0));
        }
      }
    }
  }
}

TEST(test_streaming_code,
    test_add_pkt_is_stripe_recovered_recovered_stripe)
{
  srand(0);
  for (uint16_t tau = 0; tau < 4; tau++) {
    for (uint16_t trial = 0; trial < 20; trial++) {
      for (uint16_t size_factor = 1; size_factor < 2; size_factor++) {
        uint16_t w = 32;
        uint16_t max_frame_size_possible = 10000;
        uint16_t packet_size = 8 * size_factor;
        uint16_t stripe_size = packet_size * w;
        uint16_t max_data_frame_pkts = 100;
        uint16_t max_fec_frame_pkts = 50;
        uint16_t max_data_stripes_per_frame = 50;
        uint16_t max_fec_stripes_per_frame = 25;
        assert (max_data_stripes_per_frame * stripe_size >= max_frame_size_possible);
        assert (max_fec_stripes_per_frame * stripe_size >= max_frame_size_possible / 2);
        assert (pow(2, w) >= uint64_t(num_frames_for_delay(tau)) * uint64_t(num_frames_for_delay(tau))
            * max_data_stripes_per_frame * max_fec_stripes_per_frame);
        CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(tau) *
            max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * max_data_stripes_per_frame), w};
        for (uint16_t stripes_per_pkt = 1; stripes_per_pkt < 4; stripes_per_pkt++) {
          BlockCode blockCodeFEC(codingMatrixInfoFEC, tau,
              pair<uint16_t, uint16_t>{1, 1 }, packet_size, false);
          StreamingCode code(tau, stripe_size,  &blockCodeFEC, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);
          uint16_t frame_num = 0;
          uint16_t num_frames = num_frames_for_delay(tau);
          uint16_t num_init = rand() % (num_frames * 5) + num_frames;
          while (frame_num < num_init) {
            code.updated_ts(frame_num);
            code.set_timeslot(frame_num);
            frame_num++;
          }
          frame_num += rand() % (2 * num_frames) + 1;
          uint16_t frame_size = rand() % (max_data_stripes_per_frame * stripe_size);
          if (frame_size == 0) { frame_size++; }
          while (frame_size % stripe_size) { frame_size++; }
          while (frame_size % (stripes_per_pkt * stripe_size)) { frame_size++; }
          uint16_t num_pkts = frame_size / (stripes_per_pkt * stripe_size);
          FECDatagram pkt {0, false, frame_num, 0, 0, (rand() % num_pkts) * stripes_per_pkt,
              string(stripes_per_pkt * stripe_size, char(1)) };
          code.add_packet(pkt, frame_num);
          for (uint16_t check_frame = 0; check_frame < num_frames; check_frame++) {
            if ((check_frame % num_frames) == (frame_num % num_frames)) {
              for (uint16_t stripe = 0; stripe < compute_num_data_stripes(frame_size, stripe_size); stripe++) {
                bool is_recovered = stripe >= pkt.stripe_pos_in_frame and stripe <
                    pkt.stripe_pos_in_frame + stripes_per_pkt;
                EXPECT_EQ(code.is_stripe_recovered(check_frame, stripe,
                    frame_size), is_recovered);
                EXPECT_EQ(code.recovered_stripe(frame_size, check_frame, stripe),
                    string(final_pos_within_stripe(frame_size, stripe_size, stripe) + 1,
                    char(is_recovered? 1 : 0)));
              }
            }
            else {
              uint16_t size = rand() % (max_data_stripes_per_frame * stripe_size);
              for (uint16_t stripe = 0; stripe < compute_num_data_stripes(size, stripe_size); stripe++) {
                EXPECT_FALSE(code.is_stripe_recovered(check_frame, stripe, size));
                EXPECT_EQ(code.recovered_stripe(size, check_frame, stripe),
                    string(final_pos_within_stripe(size, stripe_size, stripe) + 1, char(0)));
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code,
    test_is_frame_recovered_expired)
{
  srand(0);
  for (uint16_t tau = 0; tau < 4; tau++) {
    for (uint16_t trial = 0; trial < 2; trial++) {
      for (uint16_t size_factor = 1; size_factor < 2; size_factor++) {
        uint16_t w = 32;
        uint16_t max_frame_size_possible = 10000;
        uint16_t packet_size = 8 * size_factor;
        uint16_t stripe_size = packet_size * w;
        uint16_t max_data_frame_pkts = 100;
        uint16_t max_fec_frame_pkts = 50;
        uint16_t max_data_stripes_per_frame = 50;
        uint16_t max_fec_stripes_per_frame = 25;
        assert (max_data_stripes_per_frame * stripe_size >= max_frame_size_possible);
        assert (max_fec_stripes_per_frame * stripe_size >= max_frame_size_possible / 2);
        assert (pow(2, w) >= uint64_t(num_frames_for_delay(tau)) * uint64_t(num_frames_for_delay(tau))
            * max_data_stripes_per_frame * max_fec_stripes_per_frame);
        CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(tau) *
            max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * max_data_stripes_per_frame), w};
        for (uint16_t start = num_frames_for_delay(tau); start < 4*num_frames_for_delay(tau); start++) {
          BlockCode blockCodeFEC(codingMatrixInfoFEC, tau,
              pair<uint16_t, uint16_t>{1, 1 }, packet_size, false);
          StreamingCode code(tau, stripe_size,  &blockCodeFEC, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);
          uint16_t frame_num = 0;
          while (frame_num <= start) {
            code.updated_ts(frame_num);
            code.set_timeslot(frame_num);
            frame_num++;
          }
          for (uint16_t t = 0; t <= start - num_frames_for_delay(tau); t++) {
            EXPECT_FALSE(code.is_frame_recovered(t, rand() % max_frame_size_possible + 1));
          }
        }
      }
    }
  }
}

TEST(test_streaming_code,
    test_is_frame_recovered_and_recovered_frame)
{
  srand(0);
  for (uint16_t tau = 0; tau < 4; tau++) {
    for (uint16_t trial = 0; trial < 2; trial++) {
      for (uint16_t size_factor = 1; size_factor < 2; size_factor++) {
        uint16_t w = 32;
        uint16_t max_frame_size_possible = 10000;
        uint16_t packet_size = 8 * size_factor;
        uint16_t stripe_size = packet_size * w;
        uint16_t max_data_frame_pkts = 100;
        uint16_t max_fec_frame_pkts = 50;
        uint16_t max_data_stripes_per_frame = 50;
        uint16_t max_fec_stripes_per_frame = 25;
        assert (max_data_stripes_per_frame * stripe_size >= max_frame_size_possible);
        assert (max_fec_stripes_per_frame * stripe_size >= max_frame_size_possible / 2);
        assert (pow(2, w) >= uint64_t(num_frames_for_delay(tau)) * uint64_t(num_frames_for_delay(tau))
            * max_data_stripes_per_frame * max_fec_stripes_per_frame);
        CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(tau) *
            max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * max_data_stripes_per_frame), w};
        for (uint16_t start = num_frames_for_delay(tau); start < 4*num_frames_for_delay(tau); start++) {
          BlockCode blockCodeFEC(codingMatrixInfoFEC, tau,
              pair<uint16_t, uint16_t>{1, 1 }, packet_size, false);
          StreamingCode code(tau, stripe_size,  &blockCodeFEC, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);
          uint16_t frame_num = 0;
          vector<uint16_t> sizes;
          vector<vector<bool>> missings;
          vector<uint8_t> num_missings;
          while (frame_num <= start) {
            code.updated_ts(frame_num);
            code.set_timeslot(frame_num);
            if (frame_num >= start - num_frames_for_delay(tau) + 1) {
              uint16_t size = rand() % (trial == 0? 2 * stripe_size :(max_data_stripes_per_frame * stripe_size)) + 1;
              uint16_t num_data_stripes = compute_num_data_stripes(size, stripe_size);
              blockCodeFEC.pad(frame_num, num_data_stripes);
              uint8_t num_missing = rand() % (num_data_stripes + 1);
              auto missing = get_missing(uint8_t(num_data_stripes), num_missing);
              sizes.push_back(size);
              missings.push_back(missing);
              num_missings.push_back(num_missing);
              for (uint16_t pos = 0; pos < num_data_stripes; pos++) {
                FECDatagram pkt {0, 0, frame_num, 0, pos, pos,
                    string(stripe_size, char(frame_num + 1))};
                if (!missing.at(pos)) { code.add_packet(pkt, frame_num); }
              }
            }
            frame_num++;
          }
          for (uint16_t t = start - num_frames_for_delay(tau) + 1; t <= start; t++) {
            uint16_t size = sizes.at(t - (start - num_frames_for_delay(tau) + 1));
            auto missing = missings.at(t - (start - num_frames_for_delay(tau) + 1));
            auto num_missing = num_missings.at(t - (start - num_frames_for_delay(tau) + 1));
            EXPECT_EQ(code.is_frame_recovered(t, size), num_missing == 0);
            if (num_missing == 0) {
              EXPECT_EQ(code.recovered_frame(t, size),string(size, char(t + 1)));
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code,
    test_pad_frames)
{
  srand(0);
  for (uint16_t tau = 0; tau < 4; tau++) {
    for (uint16_t trial = 0; trial < 2; trial++) {
      for (uint16_t size_factor = 1; size_factor < 2; size_factor++) {
        uint16_t w = 32;
        uint16_t max_frame_size_possible = 10000;
        uint16_t packet_size = 8 * size_factor;
        uint16_t stripe_size = packet_size * w;
        uint16_t max_data_frame_pkts = 100;
        uint16_t max_fec_frame_pkts = 50;
        uint16_t max_data_stripes_per_frame = 50;
        uint16_t max_fec_stripes_per_frame = 25;
        assert (max_data_stripes_per_frame * stripe_size >= max_frame_size_possible);
        assert (max_fec_stripes_per_frame * stripe_size >= max_frame_size_possible / 2);
        assert (pow(2, w) >= uint64_t(num_frames_for_delay(tau)) * uint64_t(num_frames_for_delay(tau))
            * max_data_stripes_per_frame * max_fec_stripes_per_frame);
        CodingMatrixInfo codingMatrixInfoFEC { uint16_t(num_frames_for_delay(tau) *
            max_fec_stripes_per_frame), uint16_t(num_frames_for_delay(tau) * max_data_stripes_per_frame), w};
        for (uint16_t stripes_per_pkt = 1; stripes_per_pkt < 4; stripes_per_pkt++) {
          uint16_t num_frames = num_frames_for_delay(tau);
          for (uint16_t timeslot = 0; timeslot < 3 * num_frames; timeslot++) {
            BlockCode blockCodeFEC(codingMatrixInfoFEC, tau,
                pair<uint16_t, uint16_t>{1, 1 }, packet_size, false);
            StreamingCode code(tau, stripe_size,  &blockCodeFEC, w, max_data_stripes_per_frame, max_fec_stripes_per_frame);
            uint16_t frame_num = 0;
            uint16_t num_init = rand() % (num_frames * 5) + num_frames;
            while (frame_num < num_frames) {
              code.updated_ts(frame_num);
              code.set_timeslot(frame_num);
              frame_num++;
            }
            vector<optional<uint64_t>> frame_sizes(num_frames, 0);
            vector<uint64_t> wrap_frame_sizes(num_frames, 0);
            for (uint16_t pos = 0; pos < num_frames; pos++) {
              frame_sizes.at(pos) = 1 + rand() % max_frame_size_possible;
            }
            code.pad_frames(frame_sizes, timeslot);
            for (uint16_t j = 0; j < num_frames; j++) {
              for (uint16_t stripe = 0; stripe < max_data_stripes_per_frame; stripe++) {
                uint16_t pos = max_data_stripes_per_frame * code.recovered_frame_pos_to_frame_num(timeslot, j) + stripe;
                EXPECT_EQ(blockCodeFEC.get_erased().at(pos),
                    stripe < compute_num_data_stripes(frame_sizes.at(j).value(), stripe_size));
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code, test_encode_decode_within_frame)
{
  srand(0);
  for (uint16_t packet_size = 8; packet_size < 9; packet_size+= 8) {
    for (uint16_t w = 8; w < 9; w += 8) {
      uint16_t stripe_size = packet_size * w;
      if (w == 24) { continue; }
        for (uint16_t n_data_p_f = 1; n_data_p_f < 5; n_data_p_f++) {
          for (uint16_t n_par_p_f = 1; n_par_p_f < 5; n_par_p_f++) {
          for (uint16_t delay = 0; delay < 4; delay++) {
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
                    { 1 , u }, packet_size, false);
                BlockCode decodeBlockCode(codingMatrixInfo, delay,
                    { 1 , u }, packet_size, false);

                StreamingCode encodeCode(delay, stripe_size,  &encodeBlockCode, w, n_data_p_f, n_par_p_f);
                StreamingCode decodeCode(delay, stripe_size,  &decodeBlockCode, w, n_data_p_f, n_par_p_f);
                uint16_t size = n_data_p_f * stripe_size;
                vector<uint16_t> parity_pkt_sizes(n_par_p_f, stripe_size);
                uint16_t sqn = 0;
                for (uint16_t time = 0; time < timeslot; time++) {
                  vector<FECDatagram> data_pkts;
                  for (uint16_t pos = 0; pos < n_data_p_f; pos++) {
                    auto curr_data = data.at((time % num_frames) * n_data_p_f + pos);
                    FECDatagram pkt {sqn++, false, time, 0, 0, pos,
                    string(curr_data.begin(), curr_data.end()) };
                    data_pkts.push_back(pkt);
                  }
                  encodeCode.encode(data_pkts, parity_pkt_sizes, size);
                  decodeCode.encode(data_pkts, parity_pkt_sizes, size);
                }
                if (timeslot > 0) { decodeCode.set_timeslot(timeslot - 1); }
                auto missing = get_missing(n_data_p_f, rand() % (min(n_data_p_f,
                        n_par_p_f) + 1));
                uint16_t num_missing = 0;
                vector<FECDatagram> pkts;
                for (uint16_t pos = 0; pos < n_data_p_f; pos++) {
                  auto curr_data = data.at((timeslot % num_frames) * n_data_p_f + pos);
                  FECDatagram pkt {sqn++, false, timeslot, 0, 0, pos,
                  string(curr_data.begin(), curr_data.end()) };
                  if (!missing.at(pos)) {
                    decodeCode.add_packet(pkt, timeslot);
                  }
                  else { num_missing++; }
                  pkts.push_back(pkt);
                }
                auto parities = encodeCode.encode(pkts, parity_pkt_sizes, size);
                auto missing_par = get_missing(n_par_p_f, rand() % (n_par_p_f -
                        num_missing + 1));
                for (uint16_t pos = 0; pos < n_par_p_f; pos++) {
                  if (!missing_par.at(pos)) {
                    FECDatagram parity_pkt {sqn++, true, timeslot, 0, 0, pos,
                        parities.at(pos) };
                    decodeCode.add_packet(parity_pkt, timeslot);
                  }
                }
                if (num_missing == 0) {
                  continue;
                }
                vector<optional<uint64_t>> frame_sizes;
                if (timeslot < num_frames - 1) {
                  for (uint16_t j = 0; j < (num_frames - timeslot - 1); j++) {
                    frame_sizes.push_back(optional<uint64_t>());
                  }
                }
                while (frame_sizes.size() < num_frames) {
                  frame_sizes.push_back(size);
                }
                decodeCode.decode(frame_sizes);
                EXPECT_TRUE(decodeCode.is_frame_recovered(timeslot, size));
              }
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code, test_get_recoverable_data_burst_full_recover)
{
  for (uint16_t packet_size = 8; packet_size < 9; packet_size+= 8) {
    for (uint16_t w = 8; w < 9; w += 8) {
      uint16_t stripe_size = packet_size * w;
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 1; n_data_p_f < 5; n_data_p_f++) {
        uint16_t n_par_p_f = n_data_p_f;
        uint16_t delay = 3;
        auto num_frames = num_frames_for_delay(delay);
        for (uint16_t exp_loss = 0; exp_loss < 4; exp_loss++) {
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
                { 1 , 0 }, packet_size, false);
            BlockCode decodeBlockCode(codingMatrixInfo, delay,
                { 1 , 0 }, packet_size, false);
            StreamingCode encodeCode(delay, stripe_size,  &encodeBlockCode, w, n_data_p_f, n_par_p_f);
            StreamingCode decodeCode(delay, stripe_size,  &decodeBlockCode, w, n_data_p_f, n_par_p_f);
            uint16_t size = n_data_p_f * stripe_size;
            vector<uint16_t> parity_pkt_sizes(n_par_p_f, stripe_size);
            uint16_t sqn = 0;
            for (uint16_t time = 0; time <= timeslot; time++) {
              vector<FECDatagram> data_pkts;
              for (uint16_t pos = 0; pos < n_data_p_f; pos++) {
                auto curr_data = data.at((time % num_frames) * n_data_p_f + pos);
                FECDatagram pkt {sqn++, false, time, 0, 0, pos,
                    string(curr_data.begin(), curr_data.end()) };
                data_pkts.push_back(pkt);
              }
              auto parities = encodeCode.encode(data_pkts, parity_pkt_sizes, size);

              bool skip_0 = (time % num_frames) == ((timeslot - shift) %
                  num_frames )and (exp_loss % 2);
              bool skip_1 = (time % num_frames) == ((timeslot - shift + 1)
                  % num_frames) and (exp_loss / 2);

              if (time % num_frames != ((timeslot - shift + 3) % num_frames)
                  and time % num_frames != ((timeslot - shift + 4)
                  % num_frames)) {
                if (!skip_0 and !skip_1) {
                  for (uint16_t i = 0; i < n_data_p_f; i++) {
                    decodeCode.add_packet(data_pkts.at(i), time);
                  }
                }
              }
              for (uint16_t pos = 0; pos < n_par_p_f; pos++) {
                if (!skip_0 and !skip_1 and (time % num_frames != ((timeslot -
                    shift + 3) % num_frames) and time % num_frames != (timeslot
                    - shift + 4) % num_frames)) {
                  FECDatagram parity_pkt {sqn++, true, time, 0, 0, pos, parities.at(pos) };
                  decodeCode.add_packet(parity_pkt, time);
                }
              }
            }
            if (timeslot > 0) { decodeCode.set_timeslot(timeslot - 1); }
            vector<optional<uint64_t>> frame_sizes;
            if (timeslot < num_frames - 1) {
              for (uint16_t j = 0; j < (num_frames - timeslot - 1); j++) {
                frame_sizes.push_back(optional<uint64_t>());
              }
            }
            while (frame_sizes.size() < num_frames) {
              frame_sizes.push_back(size);
            }
            decodeCode.decode(frame_sizes);
            for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
              auto frame = frame_of_col(pos, delay, codingMatrixInfo);
              bool lost = ((frame % num_frames) == ((timeslot - shift) %
                  num_frames) and (exp_loss % 2)) or ((frame % num_frames) == (
                  (timeslot - shift + 1) % num_frames) and (exp_loss / 2));
              EXPECT_EQ(decodeCode.is_stripe_recovered(frame, pos - frame * n_data_p_f, size), !lost);
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code, test_get_recoverable_data_burst_partial_recover)
{
  for (uint16_t packet_size = 8; packet_size < 9; packet_size+= 8) {
    for (uint16_t w = 8; w < 9; w += 8) {
      uint16_t stripe_size = packet_size * w;
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 2; n_data_p_f < 10; n_data_p_f+= 2) {
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
                { 1 , 1 }, packet_size, false);
          BlockCode decodeBlockCode(codingMatrixInfo, delay,
              { 1 , 1 }, packet_size, false);
          StreamingCode encodeCode(delay, stripe_size,  &encodeBlockCode, w, n_data_p_f, n_par_p_f);
          StreamingCode decodeCode(delay, stripe_size,  &decodeBlockCode, w, n_data_p_f, n_par_p_f);
          uint16_t size = n_data_p_f * stripe_size;
          vector<uint16_t> parity_pkt_sizes(n_par_p_f, stripe_size);
          uint16_t sqn = 0;
          for (uint16_t time = 0; time <= timeslot; time++) {
            vector<FECDatagram> data_pkts;
            for (uint16_t pos = 0; pos < n_data_p_f; pos++) {
              auto curr_data = data.at((time % num_frames) * n_data_p_f + pos);
              FECDatagram pkt {sqn++, false, time, 0, 0, pos,
                  string(curr_data.begin(), curr_data.end()) };
              data_pkts.push_back(pkt);
            }
            auto parities = encodeCode.encode(data_pkts, parity_pkt_sizes, size);

            if (time % num_frames != ((timeslot - shift + 3) % num_frames)
                and time % num_frames != ((timeslot - shift + 4)
                % num_frames)) {
              for (uint16_t i = 0; i < n_data_p_f; i++) {
                decodeCode.add_packet(data_pkts.at(i), time);
              }
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
            for (uint16_t pos = 0; pos < n_par_p_f; pos++) {
              if (time % num_frames != ((timeslot - shift + 3) % num_frames)
                  and time % num_frames != ((timeslot - shift + 4)
                  % num_frames)) {
                FECDatagram parity_pkt {sqn++, true, time, 0, 0, pos,
                    parities.at(pos) };
                decodeCode.add_packet(parity_pkt, time);
              }
            }
          }
          if (timeslot > 0) { decodeCode.set_timeslot(timeslot - 1); }

          vector<optional<uint64_t>> frame_sizes;
          if (timeslot < num_frames - 1) {
            for (uint16_t j = 0; j < (num_frames - timeslot - 1); j++) {
              frame_sizes.push_back(optional<uint64_t>());
            }
          }
          while (frame_sizes.size() < num_frames) {
            frame_sizes.push_back(size);
          }
          decodeCode.decode(frame_sizes);
          StreamingCodeHelper helper(codingMatrixInfo, delay, { 1, 1 });
          for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
            auto frame = frame_of_col(pos, delay, codingMatrixInfo);
            for (uint16_t i = 0; i < packet_size * w; i++) {
              bool u_lost = frame == ((timeslot - shift + 4) % num_frames)
                  and helper.get_positions_of_us().at(pos);
              EXPECT_EQ(decodeCode.is_stripe_recovered(frame, pos - frame * n_data_p_f, size), !u_lost);
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code, test_get_recoverable_data_burst_partial_recover_exp_plus)
{
  for (uint16_t packet_size = 8; packet_size < 24; packet_size+= 8) {
    for (uint16_t w = 8; w < 9; w += 8) {
      uint16_t stripe_size = packet_size * w;
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 2; n_data_p_f < 10; n_data_p_f+= 2) {
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
                { 1 , 1 }, packet_size, false);
          BlockCode decodeBlockCode(codingMatrixInfo, delay,
              { 1 , 1 }, packet_size, false);
          StreamingCode encodeCode(delay, stripe_size,  &encodeBlockCode, w, n_data_p_f, n_par_p_f);
          StreamingCode decodeCode(delay, stripe_size,  &decodeBlockCode, w, n_data_p_f, n_par_p_f);
          uint16_t size = n_data_p_f * stripe_size;
          vector<uint16_t> parity_pkt_sizes(n_par_p_f, stripe_size);
          uint16_t sqn = 0;
          for (uint16_t time = 0; time <= timeslot; time++) {
            vector<FECDatagram> data_pkts;
            for (uint16_t pos = 0; pos < n_data_p_f; pos++) {
              auto curr_data = data.at((time % num_frames) * n_data_p_f + pos);
              FECDatagram pkt {sqn++, false, time, 0, 0, pos,
                  string(curr_data.begin(), curr_data.end()) };
              data_pkts.push_back(pkt);
            }
            auto parities = encodeCode.encode(data_pkts, parity_pkt_sizes, size);


            if (time % num_frames != ((timeslot - shift + 3) % num_frames)
                and (time % num_frames) != ((timeslot - shift ) % num_frames)
                and (time % num_frames) != ((timeslot - shift + 1) % num_frames)) {
              for (uint16_t i = 0; i < n_data_p_f; i++) {
                decodeCode.add_packet(data_pkts.at(i), time);
              }
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
          if (timeslot > 0) { decodeCode.set_timeslot(timeslot - 1); }
          vector<optional<uint64_t>> frame_sizes;
          if (timeslot < num_frames - 1) {
            for (uint16_t j = 0; j < (num_frames - timeslot - 1); j++) {
              frame_sizes.push_back(optional<uint64_t>());
            }
          }
          while (frame_sizes.size() < num_frames) {
            frame_sizes.push_back(size);
          }
          decodeCode.decode(frame_sizes);
          for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
            auto frame = frame_of_col(pos, delay, codingMatrixInfo);
            for (uint16_t i = 0; i < packet_size * w; i++) {
              bool lost = frame == ((timeslot - shift + 0) % num_frames);
              EXPECT_EQ(decodeCode.is_stripe_recovered(frame, pos - frame * n_data_p_f, size), !lost);
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code, test_get_recoverable_data_burst_full_recover_exp_plus)
{
  for (uint16_t packet_size = 8; packet_size < 9; packet_size+= 8) {
    for (uint16_t w = 8; w < 9; w += 8) {
      uint16_t stripe_size = packet_size * w;
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 2; n_data_p_f < 10; n_data_p_f+= 2) {
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
                { 1 , 1 }, packet_size, false);
          BlockCode decodeBlockCode(codingMatrixInfo, delay,
              { 1 , 1 }, packet_size, false);
          StreamingCode encodeCode(delay, stripe_size,  &encodeBlockCode, w, n_data_p_f, n_par_p_f);
          StreamingCode decodeCode(delay, stripe_size,  &decodeBlockCode, w, n_data_p_f, n_par_p_f);
          uint16_t size = n_data_p_f * stripe_size;
          vector<uint16_t> parity_pkt_sizes(n_par_p_f, stripe_size);
          uint16_t sqn = 0;
          for (uint16_t time = 0; time <= timeslot; time++) {
            vector<FECDatagram> data_pkts;
            for (uint16_t pos = 0; pos < n_data_p_f; pos++) {
              auto curr_data = data.at((time % num_frames) * n_data_p_f + pos);
              FECDatagram pkt {sqn++, false, time, 0, 0, pos,
                  string(curr_data.begin(), curr_data.end()) };
              data_pkts.push_back(pkt);
            }
            auto parities = encodeCode.encode(data_pkts, parity_pkt_sizes, size);


            if (time % num_frames != ((timeslot - shift + 2) % num_frames)
                and (time % num_frames) != ((timeslot - shift + 3) %
                num_frames) and (time % num_frames) != ((timeslot - shift +
                4) % num_frames)) {
              for (uint16_t i = 0; i < n_data_p_f; i++) {
                decodeCode.add_packet(data_pkts.at(i), time);
              }
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
          if (timeslot > 0) { decodeCode.set_timeslot(timeslot - 1); }
          vector<optional<uint64_t>> frame_sizes;
          if (timeslot < num_frames - 1) {
            for (uint16_t j = 0; j < (num_frames - timeslot - 1); j++) {
              frame_sizes.push_back(optional<uint64_t>());
            }
          }
          while (frame_sizes.size() < num_frames) {
            frame_sizes.push_back(size);
          }
          decodeCode.decode(frame_sizes);
          for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
            auto frame = frame_of_col(pos, delay, codingMatrixInfo);
            for (uint16_t i = 0; i < packet_size * w; i++) {
              bool u_lost = frame == ((timeslot - shift + 4) % num_frames)
                  and helper.get_positions_of_us().at(pos);
              EXPECT_EQ(decodeCode.is_stripe_recovered(frame, pos - frame * n_data_p_f, size), !u_lost);
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code, test_get_recoverable_data_burst_v_recover)
{
  for (uint16_t packet_size = 8; packet_size < 9; packet_size+= 8) {
    for (uint16_t w = 8; w < 9; w += 8) {
      uint16_t stripe_size = packet_size * w;
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 2; n_data_p_f < 9; n_data_p_f+= 2) {
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
                { 1 , 1 }, packet_size, false);
          BlockCode decodeBlockCode(codingMatrixInfo, delay,
              { 1 , 1 }, packet_size, false);
          StreamingCode encodeCode(delay, stripe_size,  &encodeBlockCode, w, n_data_p_f, n_par_p_f);
          StreamingCode decodeCode(delay, stripe_size,  &decodeBlockCode, w, n_data_p_f, n_par_p_f);
          uint16_t size = n_data_p_f * stripe_size;
          vector<uint16_t> parity_pkt_sizes(n_par_p_f, stripe_size);
          uint16_t sqn = 0;
          for (uint16_t time = 0; time <= timeslot; time++) {
            vector<FECDatagram> data_pkts;
            for (uint16_t pos = 0; pos < n_data_p_f; pos++) {
              auto curr_data = data.at((time % num_frames) * n_data_p_f + pos);
              FECDatagram pkt {sqn++, false, time, 0, 0, pos,
                  string(curr_data.begin(), curr_data.end()) };
              data_pkts.push_back(pkt);
            }
            auto parities = encodeCode.encode(data_pkts, parity_pkt_sizes, size);

            if (time % num_frames != ((timeslot - shift + 4) % num_frames)
                and time % num_frames != ((timeslot - shift + 5)
                % num_frames)) {
              for (uint16_t i = 0; i < n_data_p_f; i++) {
                decodeCode.add_packet(data_pkts.at(i), time);
              }
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
          if (timeslot > 0) { decodeCode.set_timeslot(timeslot - 1); }
          vector<optional<uint64_t>> frame_sizes;
          if (timeslot < num_frames - 1) {
            for (uint16_t j = 0; j < (num_frames - timeslot - 1); j++) {
              frame_sizes.push_back(optional<uint64_t>());
            }
          }
          while (frame_sizes.size() < num_frames) {
            frame_sizes.push_back(size);
          }
          decodeCode.decode(frame_sizes);
          for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
            auto frame = frame_of_col(pos, delay, codingMatrixInfo);
            for (uint16_t i = 0; i < packet_size * w; i++) {
              bool u_lost = ((frame == ((timeslot - shift + 4) % num_frames))
                  or (frame == ((timeslot - shift + 5) % num_frames)))
                  and helper.get_positions_of_us().at(pos);
              EXPECT_EQ(decodeCode.is_stripe_recovered(frame, pos - frame * n_data_p_f, size), !u_lost);
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code, test_get_recoverable_data_burst_not_recovered)
{
  for (uint16_t packet_size = 8; packet_size < 9; packet_size+= 8) {
    for (uint16_t w = 8; w < 9; w += 8) {
      uint16_t stripe_size = packet_size * w;
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 2; n_data_p_f < 9; n_data_p_f+= 2) {
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
                { 1 , 1 }, packet_size, false);
          BlockCode decodeBlockCode(codingMatrixInfo, delay,
              { 1 , 1 }, packet_size, false);
          StreamingCode encodeCode(delay, stripe_size,  &encodeBlockCode, w, n_data_p_f, n_par_p_f);
          StreamingCode decodeCode(delay, stripe_size,  &decodeBlockCode, w, n_data_p_f, n_par_p_f);
          uint16_t size = n_data_p_f * stripe_size;
          vector<uint16_t> parity_pkt_sizes(n_par_p_f, stripe_size);
          uint16_t sqn = 0;
          for (uint16_t time = 0; time <= timeslot; time++) {
            vector<FECDatagram> data_pkts;
            for (uint16_t pos = 0; pos < n_data_p_f; pos++) {
              auto curr_data = data.at((time % num_frames) * n_data_p_f + pos);
              FECDatagram pkt {sqn++, false, time, 0, 0, pos,
                  string(curr_data.begin(), curr_data.end()) };
              data_pkts.push_back(pkt);
            }
            auto parities = encodeCode.encode(data_pkts, parity_pkt_sizes, size);

            if (time % num_frames != ((timeslot - shift + 3) % num_frames)
                and time % num_frames != ((timeslot - shift + 4)
                % num_frames)) {
              for (uint16_t i = 0; i < n_data_p_f; i++) {
                decodeCode.add_packet(data_pkts.at(i), time);
              }
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
          if (timeslot > 0) { decodeCode.set_timeslot(timeslot - 1); }
          vector<optional<uint64_t>> frame_sizes;
          if (timeslot < num_frames - 1) {
            for (uint16_t j = 0; j < (num_frames - timeslot - 1); j++) {
              frame_sizes.push_back(optional<uint64_t>());
            }
          }
          while (frame_sizes.size() < num_frames) {
            frame_sizes.push_back(size);
          }
          decodeCode.decode(frame_sizes);
          for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
            auto frame = frame_of_col(pos, delay, codingMatrixInfo);
            for (uint16_t i = 0; i < packet_size * w; i++) {
              bool lost = ((frame == ((timeslot - shift + 3) % num_frames))
                  or (frame == ((timeslot - shift + 4) % num_frames)));
              EXPECT_EQ(decodeCode.is_stripe_recovered(frame, pos - frame * n_data_p_f, size), !lost);
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code, test_get_recoverable_data_burst_full_recover_exp)
{
  for (uint16_t packet_size = 8; packet_size < 9; packet_size+= 8) {
    for (uint16_t w = 8; w < 9; w += 8) {
      uint16_t stripe_size = packet_size * w;
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 1; n_data_p_f < 5; n_data_p_f++) {
        uint16_t n_par_p_f = n_data_p_f;
        uint16_t delay = 3;
        auto num_frames = num_frames_for_delay(delay);
        for (uint16_t shift_start = 3; shift_start > 0; shift_start--) {
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
                { 1 , 0 }, packet_size, false);
            BlockCode decodeBlockCode(codingMatrixInfo, delay,
                { 1 , 0 }, packet_size, false);
            StreamingCode encodeCode(delay, stripe_size,  &encodeBlockCode, w, n_data_p_f, n_par_p_f);
            StreamingCode decodeCode(delay, stripe_size,  &decodeBlockCode, w, n_data_p_f, n_par_p_f);
            uint16_t size = n_data_p_f * stripe_size;
            vector<uint16_t> parity_pkt_sizes(n_par_p_f, stripe_size);
            uint16_t sqn = 0;
            for (uint16_t time = 0; time <= timeslot; time++) {
              vector<FECDatagram> data_pkts;
              for (uint16_t pos = 0; pos < n_data_p_f; pos++) {
                auto curr_data = data.at((time % num_frames) * n_data_p_f + pos);
                FECDatagram pkt {sqn++, false, time, 0, 0, pos,
                    string(curr_data.begin(), curr_data.end()) };
                data_pkts.push_back(pkt);
              }
              auto parities = encodeCode.encode(data_pkts, parity_pkt_sizes, size);

              if (time % num_frames != ((timeslot - shift + 3 - shift_start)
                  % num_frames) and time % num_frames != ((timeslot - shift +
                  4 - shift_start) % num_frames)) {
                for (uint16_t i = 0; i < n_data_p_f; i++) {
                  decodeCode.add_packet(data_pkts.at(i), time);
                }
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
            if (timeslot > 0) { decodeCode.set_timeslot(timeslot - 1); }
            vector<optional<uint64_t>> frame_sizes;
            if (timeslot < num_frames - 1) {
              for (uint16_t j = 0; j < (num_frames - timeslot - 1); j++) {
                frame_sizes.push_back(optional<uint64_t>());
              }
            }
            while (frame_sizes.size() < num_frames) {
              frame_sizes.push_back(size);
            }
            decodeCode.decode(frame_sizes);
            for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
              auto frame = frame_of_col(pos, delay, codingMatrixInfo);
              EXPECT_TRUE(decodeCode.is_stripe_recovered(frame, pos - frame * n_data_p_f, size));
            }
          }
        }
      }
    }
  }
}

TEST(test_streaming_code, test_get_recoverable_data_burst_not_recovered_exp)
{
  for (uint16_t packet_size = 8; packet_size < 9; packet_size+= 8) {
    for (uint16_t w = 8; w < 9; w += 8) {
      uint16_t stripe_size = packet_size * w;
      if (w == 24) { continue; }
      for (uint16_t n_data_p_f = 2; n_data_p_f < 9; n_data_p_f+= 2) {
        uint16_t n_par_p_f = n_data_p_f / 2;
        uint16_t delay = 3;
        auto num_frames = num_frames_for_delay(delay);
        for (uint16_t shift_start = 3; shift_start > 0; shift_start--) {
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
                { 1 , 1 }, packet_size, false);
            BlockCode decodeBlockCode(codingMatrixInfo, delay,
                { 1 , 1 }, packet_size, false);
            StreamingCode encodeCode(delay, stripe_size,  &encodeBlockCode, w, n_data_p_f, n_par_p_f);
            StreamingCode decodeCode(delay, stripe_size,  &decodeBlockCode, w, n_data_p_f, n_par_p_f);
            uint16_t size = n_data_p_f * stripe_size;
            vector<uint16_t> parity_pkt_sizes(n_par_p_f, stripe_size);
            uint16_t sqn = 0;
            for (uint16_t time = 0; time <= timeslot; time++) {
              vector<FECDatagram> data_pkts;
              for (uint16_t pos = 0; pos < n_data_p_f; pos++) {
                auto curr_data = data.at((time % num_frames) * n_data_p_f + pos);
                FECDatagram pkt {sqn++, false, time, 0, 0, pos,
                    string(curr_data.begin(), curr_data.end()) };
                data_pkts.push_back(pkt);
              }
              auto parities = encodeCode.encode(data_pkts, parity_pkt_sizes, size);

              if (time % num_frames != ((timeslot - shift + 3 - shift_start)
                  % num_frames) and time % num_frames != ((timeslot - shift +
                  4 - shift_start) % num_frames)) {
                for (uint16_t i = 0; i < n_data_p_f; i++) {
                  decodeCode.add_packet(data_pkts.at(i), time);
                }
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
            if (timeslot > 0) { decodeCode.set_timeslot(timeslot - 1); }
            vector<optional<uint64_t>> frame_sizes;
            if (timeslot < num_frames - 1) {
              for (uint16_t j = 0; j < (num_frames - timeslot - 1); j++) {
                frame_sizes.push_back(optional<uint64_t>());
              }
            }
            while (frame_sizes.size() < num_frames) {
              frame_sizes.push_back(size);
            }
            decodeCode.decode(frame_sizes);
            for (uint16_t pos = 0; pos < num_frames * n_data_p_f; pos++) {
              auto frame = frame_of_col(pos, delay, codingMatrixInfo);
              for (uint16_t i = 0; i < packet_size * w; i++) {
                bool lost = ((frame == ((timeslot - shift + 3 - shift_start) %
                    num_frames)) or (frame == ((timeslot - shift + 4 -
                    shift_start) % num_frames)));
                EXPECT_EQ(decodeCode.is_stripe_recovered(frame, pos - frame * n_data_p_f, size), !lost);
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
