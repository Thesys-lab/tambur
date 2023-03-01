#include "gtest/gtest.h"

#include "serialization.hh"
#include "helpers.hh"

using namespace std;

TEST(test_frame_generator, generate_frame_pkts_RS_packetization)
{
  Packetization * packetization = get_Packetization(0, 8*32, 8);
  auto frameGenerator = get_FrameGenerator();
  uint16_t video_frame_num = 0;
  uint8_t frameType = 0;
  uint8_t num_frames_in_video_frame = 1;
  for (uint16_t size = 1000; size < 1100; size++) {
    for (int state = 0 ; state < 2; state += 1) {
      frameGenerator.update_state(state);
      const auto pkts = frameGenerator.generate_frame_pkts(size,
          video_frame_num, frameType,
            num_frames_in_video_frame);
      video_frame_num += 2;
      frameType += 3;
      num_frames_in_video_frame += 4;
      auto RS_packets = packetization->get_packetization(size, state);
      EXPECT_EQ(RS_packets.size(), pkts.size());
      for (uint16_t i = 0; i < RS_packets.size(); i++) {
        const auto pkt = pkts.at(i);
        const auto pkt_packetization_info = RS_packets.at(i);
        EXPECT_EQ(pkt_packetization_info.first, pkt.payload.size());
        EXPECT_EQ(pkt_packetization_info.second, pkt.is_parity);
      }
    }
  }
}

TEST(test_frame_generator, generate_frame_pkts_RS_header_info)
{
  uint16_t w = 32;
  uint16_t packet_size = 8;
  uint16_t stripe_size = packet_size * w;
  uint16_t max_data_stripes_per_frame = 50;
  uint16_t max_fec_stripes_per_frame = 50;
  uint16_t tau = 0;
  uint16_t num_frames = num_frames_for_delay(tau);
  uint16_t n_cols = num_frames;
  uint16_t n_rows = 256;
  while (n_rows * n_cols > 255 or (n_rows % num_frames)) { n_rows--; }
  CodingMatrixInfo codingMatrixInfoHeader { n_rows, n_cols, 8};

  BlockCode blockCodeHeaderSender(codingMatrixInfoHeader, tau,
      pair<uint16_t, uint16_t>{1, 0 }, 8, false);
  MultiFECHeaderCode headerCode(&blockCodeHeaderSender, tau,
      codingMatrixInfoHeader.n_rows / num_frames_for_delay(tau));
  auto frameGenerator = get_FrameGenerator();
  uint16_t video_frame_num = 0;
  uint8_t frameType = 0;
  uint8_t num_frames_in_video_frame = 1;
  uint16_t sqn_num = 0;
  uint16_t frame_num = 0;
  uint8_t val = 1;
  for (uint16_t random_str = 0; random_str < 2; random_str++) {
    uint8_t * ptr = random_str ? &val : nullptr;
    for (uint64_t size = 100; size < 200; size++) {
      vector<uint64_t> frame_sizes = { size };
      for (int state = 0 ; state < 2; state += 1) {
        frameGenerator.update_state(state);
        const auto pkts = frameGenerator.generate_frame_pkts(size,
            video_frame_num, frameType,
            num_frames_in_video_frame);
        video_frame_num += 2;
        frameType += 3;
        num_frames_in_video_frame += 4;
        for (uint16_t i = 0; i < pkts.size(); i++) {
          auto encoded_frame_size = headerCode.encode_sizes_of_frames(frame_sizes.back(),
              i);
          const auto pkt = pkts.at(i);
          EXPECT_EQ(pkt.sizes_of_frames_encoding, encoded_frame_size);
          EXPECT_EQ(pkt.pos_in_frame, i);
          EXPECT_EQ(pkt.seq_num, sqn_num + i);
          EXPECT_EQ(pkt.frame_num, frame_num);
        }
        sqn_num = (sqn_num + pkts.size());
        frame_num = (frame_num + 1) % (65536 / 2);
      }
    }
  }
}


TEST(test_frame_generator, generate_frame_pkts_given_payload)
{
  auto frameGenerator = get_FrameGenerator();
  uint32_t video_frame_num = 0;
  uint8_t frameType = 0;
  uint8_t num_frames_in_video_frame = 1;
  uint8_t buff[11006];
  for (uint16_t j = 0; j < 11006; j++) {
    buff[j] = j % 8;
  }
  for (uint16_t size = 1000; size < 11000; size += 131) {
    for (int state = 0 ; state < 2; state += 1) {
      uint16_t total = 0;
      frameGenerator.update_state(state);
      video_frame_num += 2;
      frameType += 3;
      num_frames_in_video_frame += 4;
      for (uint16_t j = 0; j < size - FECDatagram::VIDEO_FRAME_INFO_SIZE; j++) {
        buff[j] = j % 8;
      }
      string tmp = put_number(video_frame_num);
      buff[size - FECDatagram::VIDEO_FRAME_INFO_SIZE] = frameType;
      buff[size - FECDatagram::VIDEO_FRAME_INFO_SIZE + 1] = num_frames_in_video_frame;
      for (uint8_t j = 0; j < 4; j++) {
        buff[size - FECDatagram::VIDEO_FRAME_INFO_SIZE + j + 2] = (uint8_t) tmp[j];
      }
      const auto pkts = frameGenerator.generate_frame_pkts(size,
          video_frame_num, frameType,
          num_frames_in_video_frame, buff);
      for (uint16_t pos = 0; pos < pkts.size(); pos++) {
        const auto pkt = pkts.at(pos);
        if (pkt.is_parity) { continue; }
        auto sz = pkts.front().payload.size();
        for (uint16_t p = sz * pos; p < sz * (pos + 1); p++) {
          EXPECT_EQ((uint8_t) pkt.payload.at(p - sz * pos), p >= size ?
              uint8_t(char(0)): buff[p]);
        }
        total += pkt.payload.size();
      }
      uint16_t padded_size = size;
      while (padded_size % 256) { padded_size++; }
      EXPECT_EQ(total, padded_size);
    }
  }
}


TEST(test_frame_generator, generate_frame_pkts_RS_same_seed)
{
  auto firstFrameGenerator = get_FrameGenerator();
  auto secondFrameGenerator = get_FrameGenerator();
  uint16_t video_frame_num = 0;
  uint8_t frameType = 0;
  uint8_t num_frames_in_video_frame = 1;
  for (uint16_t size = 1000; size < 1100; size++) {
    for (int state = 0 ; state < 2; state += 1) {
      firstFrameGenerator.update_state(state);
      secondFrameGenerator.update_state(state);
      const auto first_pkts = firstFrameGenerator.generate_frame_pkts(size,
          video_frame_num, frameType,
          num_frames_in_video_frame);

      const auto second_pkts = secondFrameGenerator.generate_frame_pkts(size,
          video_frame_num, frameType,
          num_frames_in_video_frame);
      video_frame_num += 2;
      frameType += 3;
      num_frames_in_video_frame += 4;
      EXPECT_EQ(first_pkts.size(), second_pkts.size());
      for (uint16_t pos = 0; pos < first_pkts.size(); pos++) {
        const auto first_pkt = first_pkts.at(pos);
        const auto second_pkt = second_pkts.at(pos);
        EXPECT_TRUE(first_pkt.payload == second_pkt.payload);
        EXPECT_EQ(first_pkt.frame_num, second_pkt.frame_num);
        EXPECT_EQ(first_pkt.is_parity, second_pkt.is_parity);
        EXPECT_EQ(first_pkt.pos_in_frame,
            second_pkt.pos_in_frame);
        EXPECT_EQ(first_pkt.seq_num, second_pkt.seq_num);
        EXPECT_EQ(first_pkt.sizes_of_frames_encoding,
            second_pkt.sizes_of_frames_encoding);
      }
    }
  }
}

TEST(test_frame_generator, generate_frame_pkts_RS_diff_seed)
{
  auto firstFrameGenerator = get_FrameGenerator(0,1);
  auto secondFrameGenerator = get_FrameGenerator(0,2);
  uint16_t video_frame_num = 0;
  uint8_t frameType = 0;
  uint8_t num_frames_in_video_frame = 1;
  for (uint16_t size = 1000; size < 1100; size++) {
    for (int state = 0 ; state < 2; state += 1) {
      firstFrameGenerator.update_state(state);
      secondFrameGenerator.update_state(state);
      const auto first_pkts = firstFrameGenerator.generate_frame_pkts(size,
          video_frame_num, frameType,
          num_frames_in_video_frame);
      const auto second_pkts = secondFrameGenerator.generate_frame_pkts(size,
          video_frame_num, frameType,
          num_frames_in_video_frame);
      video_frame_num += 2;
      frameType += 3;
      num_frames_in_video_frame += 4;
      for (uint16_t pos = 0; pos < first_pkts.size(); pos++) {
        const auto first_pkt = first_pkts.at(pos);
        const auto second_pkt = second_pkts.at(pos);
        EXPECT_TRUE(first_pkt.payload != second_pkt.payload);
        EXPECT_EQ(first_pkt.frame_num, second_pkt.frame_num);
        EXPECT_EQ(first_pkt.is_parity, second_pkt.is_parity);
        EXPECT_EQ(first_pkt.pos_in_frame,
            second_pkt.pos_in_frame);
        EXPECT_EQ(first_pkt.seq_num, second_pkt.seq_num);
        EXPECT_EQ(first_pkt.sizes_of_frames_encoding,
            second_pkt.sizes_of_frames_encoding);
      }
    }
  }
}


TEST(test_frame_generator, generate_frame_pkts_letter_frequency)
{
  srand(0);
  auto frameGenerator = get_FrameGenerator();
  uint16_t video_frame_num = 0;
  uint8_t frameType = 0;
  uint8_t num_frames_in_video_frame = 1;
  uint16_t max_num_data_pkts = 150;
  vector<vector<uint16_t>> frequencies;
  frequencies.reserve(max_num_data_pkts);
  for (uint16_t pos = 0; pos < max_num_data_pkts; pos++) {
    vector<uint16_t> char_frequencies(256, 0);
    frequencies.push_back(char_frequencies);
  }
  int threshold = 100*256;
  uint16_t num_trials = 100;
  for (uint16_t trial = 0; trial < num_trials; trial++) {
    for (uint16_t size = 1000; size < 1010; size++) {
      for (int state = 0 ; state < 2; state += 1) {
        frameGenerator.update_state(state);
        const auto pkts = frameGenerator.generate_frame_pkts(size,
            video_frame_num, frameType,
            num_frames_in_video_frame);
        video_frame_num += 2;
        frameType += 3;
        num_frames_in_video_frame += 4;

        for (uint16_t i = 0; i < pkts.size(); i++) {
          const auto pkt = pkts.at(i);
          if (!pkt.is_parity) {
            vector<uint16_t> char_frequencies(256, 0);
            for (uint16_t pos = 0; pos < pkt.payload.size(); pos++) {
              char_frequencies[uint8_t(pkt.payload[pos])]++;
            }
            for (uint16_t index = 0; index < 256; index++) {
              frequencies.at(i).at(index) += char_frequencies.at(index);
            }
          }
        }
      }
    }
  }
  for (uint16_t pos = 0; pos < frequencies.size(); pos++) {
    const auto pkt_frequencies = frequencies.at(pos);
    float total = 0;
    for (uint16_t char_frequency = 0; char_frequency < 256; char_frequency++) {
      total += int(pkt_frequencies.at(char_frequency));
    }
    if (total >= threshold) {
      for (uint16_t char_frequency = 0; char_frequency < 256;
          char_frequency++) {
        const auto freq = float(pkt_frequencies.at(char_frequency));
        const auto expected_freq = total / 256.0;
        EXPECT_GE(freq, expected_freq / 1.25);
        if (char_frequency > 0) {
          EXPECT_LE(freq, expected_freq * 1.25);
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
