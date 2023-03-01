#include "gtest/gtest.h"

#include "fec_datagram.hh"

using namespace std;

TEST(test_fec_datagram, test_init)
{
  for (uint8_t seq_num = 0; seq_num < 2; seq_num++) {
    for (uint8_t is_parity = 0; is_parity < 2; is_parity++) {
      for (uint8_t frame_num = 0; frame_num < 2; frame_num++) {
        for (uint8_t sizes_encoding = 0; sizes_encoding < 2;
            sizes_encoding++) {
          for (uint8_t pos = 0; pos < 2; pos++) {
            for (uint8_t str_len = 0; str_len < 2; str_len++) {
              FECDatagram fECDatagram { seq_num, bool(is_parity), frame_num,
                  sizes_encoding, pos, pos + 1, string(str_len, 'x') };
              EXPECT_EQ(seq_num, fECDatagram.seq_num);
              EXPECT_EQ(is_parity, fECDatagram.is_parity);
              EXPECT_EQ(frame_num, fECDatagram.frame_num);
              EXPECT_EQ(sizes_encoding,
                  fECDatagram.sizes_of_frames_encoding);
              EXPECT_EQ(pos, fECDatagram.pos_in_frame);
              EXPECT_EQ(pos + 1 , fECDatagram.stripe_pos_in_frame);
              EXPECT_EQ(string(str_len, 'x'),  fECDatagram.payload);
            }
          }
        }
      }
    }
  }
}

TEST(test_fec_datagram, test_parse_and_serialize)
{
  FECDatagram fECDatagram { 1, false, 2, 3, 4, 0, "test" };
  FECDatagram testParseFECDatagram;
  testParseFECDatagram.parse_from_string(fECDatagram.serialize_to_string());
  EXPECT_EQ(testParseFECDatagram.seq_num, fECDatagram.seq_num);
  EXPECT_EQ(testParseFECDatagram.is_parity,
      fECDatagram.is_parity);
  EXPECT_EQ(testParseFECDatagram.frame_num,
      fECDatagram.frame_num);
  EXPECT_EQ(testParseFECDatagram.sizes_of_frames_encoding,
      fECDatagram.sizes_of_frames_encoding);
  EXPECT_EQ(testParseFECDatagram.pos_in_frame,
      fECDatagram.pos_in_frame);
  EXPECT_EQ(testParseFECDatagram.payload, fECDatagram.payload);
}

int main(int argc, char * argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
