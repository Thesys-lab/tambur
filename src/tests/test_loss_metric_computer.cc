#include "gtest/gtest.h"
#include <random>
#include <algorithm>
#include <chrono>

#include "loss_metric_computer.hh"

using namespace std;

float err = 0.00001;

TEST(test_loss_metric_computer, test_bursts_guardspaces_no_loss) {
  vector<bool> losses(20, false);
  map<uint16_t, uint16_t> bursts {};
  map<uint16_t, uint16_t> guardspaces {{20, 1}};
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test = lossMetricComputer.get_bursts_and_guardspaces(losses);
  EXPECT_EQ(bursts,test.first);
  EXPECT_EQ(guardspaces,test.second);
}

TEST(test_loss_metric_computer, test_bursts_guardspaces_all_loss) {
  vector<bool> losses (10, true);
  map<uint16_t, uint16_t> bursts {{10, 1}};
  map<uint16_t, uint16_t> guardspaces {};
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test = lossMetricComputer.get_bursts_and_guardspaces(losses);
  EXPECT_EQ(bursts,test.first);
  EXPECT_EQ(guardspaces,test.second);

}

TEST(test_loss_metric_computer, test_bursts_guardspaces_begin_burst_one_loss) {
  vector<bool> losses { true, false, false, false, false, false};
  map<uint16_t, uint16_t> bursts {{1, 1}};
  map<uint16_t, uint16_t> guardspaces {{5, 1}};
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test = lossMetricComputer.get_bursts_and_guardspaces(losses);
  EXPECT_EQ(bursts,test.first);
  EXPECT_EQ(guardspaces,test.second);

}

TEST(test_loss_metric_computer, test_bursts_guardspaces_begin_burst_multiple_losses) {
  vector<bool> losses { true, true, true, false, false, false, false, false};
  map<uint16_t, uint16_t> bursts {{3, 1}};
  map<uint16_t, uint16_t> guardspaces {{5, 1}};
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test = lossMetricComputer.get_bursts_and_guardspaces(losses);
  EXPECT_EQ(bursts,test.first);
  EXPECT_EQ(guardspaces,test.second);


}

TEST(test_loss_metric_computer, test_bursts_guardspaces_middle_burst_one_loss) {
  vector<bool> losses { false, false, false, true, false, false, false};
  map<uint16_t, uint16_t> bursts {{1, 1}};
  map<uint16_t, uint16_t> guardspaces {{3, 2}};
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test = lossMetricComputer.get_bursts_and_guardspaces(losses);
  EXPECT_EQ(bursts,test.first);
  EXPECT_EQ(guardspaces,test.second);

}

TEST(test_loss_metric_computer, test_bursts_guardspaces_middle_burst_multiple_losses) {
  vector<bool> losses { false, true, true, true, true, false, false, false, false, false};
  map<uint16_t, uint16_t> bursts {{4, 1}};
  map<uint16_t, uint16_t> guardspaces {{5, 1}, {1, 1}};
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test = lossMetricComputer.get_bursts_and_guardspaces(losses);
  EXPECT_EQ(bursts,test.first);
  EXPECT_EQ(guardspaces,test.second);
}

TEST(test_loss_metric_computer, test_bursts_guardspaces_end_burst_one_loss) {
  vector<bool> losses { false, false, false, false, false, true};
  map<uint16_t, uint16_t> bursts {{1, 1}};
  map<uint16_t, uint16_t> guardspaces {{5, 1}};
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test = lossMetricComputer.get_bursts_and_guardspaces(losses);
  EXPECT_EQ(bursts,test.first);
  EXPECT_EQ(guardspaces,test.second);
}

TEST(test_loss_metric_computer, test_bursts_guardspaces_end_burst_multiple_losses) {
  vector<bool> losses { false, true, true };
  map<uint16_t, uint16_t> bursts {{2, 1}};
  map<uint16_t, uint16_t> guardspaces {{1, 1}};
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test = lossMetricComputer.get_bursts_and_guardspaces(losses);
  EXPECT_EQ(bursts,test.first);
  EXPECT_EQ(guardspaces,test.second);
}

TEST(test_loss_metric_computer, test_bursts_guardspaces_two_bursts_middle) {
  vector<bool> losses { false, false, false, true, true, true, true, false,
      true, true, true, true, false, false, false, false, false, false, false,
      false, false, false };
  map<uint16_t, uint16_t> bursts {{4, 2}};
  map<uint16_t, uint16_t> guardspaces {{3, 1}, {1, 1}, {10, 1}};
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test = lossMetricComputer.get_bursts_and_guardspaces(losses);
  EXPECT_EQ(bursts,test.first);
  EXPECT_EQ(guardspaces,test.second);
}

TEST(test_loss_metric_computer, test_bursts_guardspaces_two_bursts_begin_end) {
  vector<bool> losses { true, true, false, false, false, false, false, false,
      false, false, true };
  map<uint16_t, uint16_t> bursts {{2, 1}, {1, 1}};
  map<uint16_t, uint16_t> guardspaces {{8, 1}};
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test = lossMetricComputer.get_bursts_and_guardspaces(losses);
  EXPECT_EQ(bursts,test.first);
  EXPECT_EQ(guardspaces,test.second);

}

TEST(test_loss_metric_computer, test_bursts_guardspaces_two_bursts_begin_middle) {
  vector<bool> losses { true, true, false, false, false, false, true, true,
      true, true, false, false, false,false, false };
  map<uint16_t, uint16_t> bursts {{2, 1}, {4, 1}};
  map<uint16_t, uint16_t> guardspaces {{5, 1}, {4, 1}};
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test = lossMetricComputer.get_bursts_and_guardspaces(losses);
  EXPECT_EQ(bursts,test.first);
  EXPECT_EQ(guardspaces,test.second);
}

TEST(test_loss_metric_computer, test_bursts_guardspaces_two_bursts_middle_end) {
  vector<bool> losses { false, false, false, false, false, true, true, true,
      false, false, false,false, false, false, true, true, true, true };
  map<uint16_t, uint16_t> bursts {{3, 1}, {4, 1}};
  map<uint16_t, uint16_t> guardspaces {{6, 1}, {5, 1}};
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test = lossMetricComputer.get_bursts_and_guardspaces(losses);
  EXPECT_EQ(bursts,test.first);
  EXPECT_EQ(guardspaces,test.second);
}

TEST(test_loss_metric_computer, test_RFC_OnlyLoss) {
  uint16_t gMin = 16;
  vector<bool> losses(100, true);
  auto test_vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  pair<float, float> targets = {1.0,0.0};
  EXPECT_EQ(targets,test_vals);
}

TEST(test_loss_metric_computer, test_RFC_NoLoss) {
  uint16_t gMin = 16;
  vector<bool> losses(100, false);
  auto test_vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  pair<float, float> targets = {0.0,0.0};
  EXPECT_EQ(targets,test_vals);
}

TEST(test_loss_metric_computer, test_RFC_TwoBurstsEdges) {
  uint16_t gMin = 16;
  vector<bool> losses {true, true, true, true, false, false, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, true, true, true, true, true};
  auto vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  EXPECT_NEAR(vals.first, 1.0, err);
  EXPECT_NEAR(vals.second, 0.0, err);
}

TEST(test_loss_metric_computer, test_RFC_BeginningBurst) {
  uint16_t gMin = 16;
  vector<bool> losses { false, false, false, false, false, false, false,
      false, true, true, true, true, true, true, false, false, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false};
  auto vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  pair<float, float> goals =  { 1.0,0.0 };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);
  losses = vector<bool> {true, true, true, true, true, true, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);
}

TEST(test_loss_metric_computer, test_RFC_EndBurst) {
  uint16_t gMin = 16;
  vector<bool> losses { false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, true, true, true,
      true, true, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false };
  auto vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  pair<float, float> goals = pair<float, float> { 1.0,0.0 };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);
  losses = vector<bool>(losses.begin(), losses.begin() + gMin + 5);
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  goals = pair<float, float> { 1.0,0.0 };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);
}

TEST(test_loss_metric_computer, test_RFC_IsolatedMiddle) {
  uint16_t gMin = 16;
  vector<bool> losses { false, false, false, false, false, false, false, false,
      true, false, false, false, false, false, false, false, false };
  auto vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  pair<float, float> goals = pair<float, float> { 0.0,1.0/float(gMin + 1) };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);
}

TEST(test_loss_metric_computer, test_RFC_IsolatedBeginning) {
  uint16_t gMin = 16;
  vector<bool> losses { false, false, false, false, false, false, false, false,
      true};
  for (uint16_t j = 0; j < 500; j++) { losses.push_back(false); }
  auto vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  pair<float, float> goals = pair<float, float> { 0.0,1.0 / float(gMin/2+500+1) };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses = vector<bool> (losses.begin() + gMin / 2, losses.end());
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  goals = pair<float, float> { 0.0,1.0 / float(500+1) };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);
}

TEST(test_loss_metric_computer, test_RFC_IsolatedEnd) {
  uint16_t gMin = 16;
  vector<bool> losses(500,false);
  losses.push_back(true);
  for (uint16_t j = 0; j < gMin / 2; j++) { losses.push_back(false); }
  auto vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  pair<float, float> goals = pair<float, float> { 0.0,1.0/float(500+1+gMin/2) };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);
  losses = vector<bool>(losses.begin(), losses.begin() + 501);
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  goals = pair<float, float> { 0.0,1.0 / 501.0 };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);
}

TEST(test_loss_metric_computer, test_RFC_BurstIsolatedBurst) {
  uint16_t gMin = 16;
  vector<bool> losses { false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, true, true, true,
      true, true, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, true, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, true, true, true, false, false, false, false,
      false };
  auto vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  pair<float, float> goals = pair<float, float> { 1.0,1.0 / float(3*gMin+6) };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);
  losses = vector<bool> { false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, true, true, true,
      true, true, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, true, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, true, true, true, false, false, false, false,
      false };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  goals = pair<float, float> { 9.0 / float(8 + gMin),0.0 };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);
  losses = vector<bool> { false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, true, true, true,
      true, true, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, true, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, true, true, true, false, false, false, false,
      false };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses  = { false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, true, true, true,
      true, true, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, true, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, true, true, true, false, false, false, false,
      false };

  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  goals = pair<float, float> { 9.0 / float(7 + 2*gMin),0.0 };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);


  losses = vector<bool> { false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, true, true, true,
      true, true, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, true, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, true, true, true, false, false, false, false,
      false };
  goals = pair<float, float> { 1.0, 1.0 / float(3*gMin + 6)};
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses = vector<bool> { false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      true, true, true, true, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, true, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, true, true, true, false, false, false,
      false, false };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  goals = pair<float, float> { 9.0 / float(8.0 + gMin),0.0 };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses = vector<bool> { false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      true, true, true, true, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, true, true, true, false, false, false,
      false, false };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  goals = pair<float, float> { 9.0 / float(8.0 + gMin),0.0 };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses = vector<bool> { false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      true, true, true, true, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, true, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, true, true, true, false, false, false, false,
      false };

  goals = pair<float, float> { 9.0 / float(7.0 + 2 * gMin), 0.0 };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  EXPECT_NEAR(vals.second,goals.second,err);
}

TEST(test_loss_metric_computer, test_RFC_IsolatedBurstIsolated) {
  uint16_t gMin = 16;
  vector<bool> losses { false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, true, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, true, true, true, true, true,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, true, false, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false };
  auto vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  pair<float, float> goals = pair<float, float> { 1.0, 1.0 / float(2*gMin+1) };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses = vector<bool> { false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, true, true, true, true, true, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, true, false, false, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  goals = pair<float, float> { 6.0 /float (gMin + 5), 1.0 / float(3 * gMin + 1) };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses = vector<bool> { false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, true, true, true, true, true,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, true,  false, false, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  goals = pair<float, float> { (1.0+5)/ float(gMin+5), 1.0 / float(3 * gMin + 1) };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses = vector<bool> { false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, true, true, true, true, true, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, true,  false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  goals = pair<float, float> { (1.0+5.0+1.0)/(gMin+5+gMin), 0.0 };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses = vector<bool> { false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, true, true, true, true,
      true, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, true, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false };
  goals = pair<float, float> { 1.0, 1.0 / float(2*gMin+1) };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses = vector<bool> { false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, true, true, true, true, true, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, true,  false, false, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false };
  goals = pair<float, float> { (1.0+5)/(gMin+5), 1.0/(gMin+gMin+1+gMin) };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses = vector<bool> { false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, true, true, true, true,
      true, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, true,  false, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false };
  goals = pair<float, float> { (1.0+5)/(gMin+5), 1.0/(gMin+gMin+1+gMin) };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses = vector<bool> { false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, true, true, true, true, true, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, true, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, false,
      false };
  goals = pair<float, float> { (1.0+5.0+1.0)/(gMin+5+gMin), 0.0 };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);
}

TEST(test_loss_metric_computer, test_RFC_TwoBurstsOneGap) {
  uint16_t gMin = 16;
  vector<bool> losses { false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, true, true, true,
      true, false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, true, true, true, true, true,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false };
      auto vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  pair<float, float> goals = pair<float, float> { 1.0,0.0 };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses  = { false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, true, true, true, true,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false, true, true, true, true, true, false,
      false, false, false, false, false, false, false, false, false, false,
      false, false, false, false, false };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  goals = pair<float, float> { 9.0 / float(8.0 + gMin), 0.0 };
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses = vector<bool> { false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      true, true, true, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      true, true, true, true, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, false };
  goals = pair<float, float> { 1.0,0.0 };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);

  losses = vector<bool> { false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, false, false, true,
      true, true, true, false, false, false, false, false, false, false, false,
      false, false, false, false, false, false, false, true, true, true, true,
      true, false, false, false, false, false,false, false, false, false,
      false, false, false, false, false, false, false };
  goals = pair<float, float> { 9.0 / float (8.0 + gMin),0.0 };
  vals = LossMetricComputer(LossInfo(), 0).getRFCDensities(losses,gMin);
  EXPECT_NEAR(vals.first,goals.first,err);
  EXPECT_NEAR(vals.second,goals.second,err);
}

TEST(test_loss_metric_computer, test_consecutive_burst_info_empty) {
  vector<bool> packet_losses;
  vector<uint16_t> frame_indices;
  uint16_t num_frames = 0;
  uint16_t num_pkts = 0;
  auto test = LossMetricComputer(LossInfo(), 0).consecutive_burst_info(
        packet_losses, frame_indices);
  EXPECT_EQ(num_frames,test.first);
  EXPECT_EQ(num_pkts,test.second);
}

TEST(test_loss_metric_computer, test_consecutive_burst_info_no_valid_positions) {
  vector<bool> packet_losses = vector<bool>(18, false);
  vector<uint16_t> frame_indices = {0, 3, 6, 10, 13, 15};
  uint16_t num_frames = 0;
  uint16_t num_pkts = 0;
  auto test = LossMetricComputer(LossInfo(), 0).consecutive_burst_info(
      packet_losses, frame_indices);
  EXPECT_EQ(num_frames,test.first);
  EXPECT_EQ(num_pkts,test.second);
}

TEST(test_loss_metric_computer, test_consecutive_burst_info_less_than_min) {
  vector<bool> packet_losses = vector<bool> {0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0};
  vector<uint16_t> frame_indices = {0, 3, 7, 10, 12 };
  uint16_t num_frames = 1;
  uint16_t num_pkts = 3;
  auto test = LossMetricComputer(LossInfo(), 0).consecutive_burst_info(
        packet_losses, frame_indices);
  EXPECT_EQ(num_frames,test.first);
  EXPECT_EQ(num_pkts,test.second);
}

TEST(test_loss_metric_computer, test_consecutive_burst_info_meet_consecutive) {
  vector<bool> packet_losses = vector<bool> {0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 0,
      0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1,
      1, 1, 1, 0, 0, 0 };
  vector<uint16_t> frame_indices = {0, 3, 7, 10, 12, 16};
  uint16_t num_frames = 3;
  uint16_t num_pkts = 10;
  auto test = LossMetricComputer(LossInfo(), 0).consecutive_burst_info(
      packet_losses, frame_indices);
  EXPECT_EQ(num_frames,test.first);
  EXPECT_EQ(num_pkts,test.second);
}

TEST(test_loss_metric_computer, test_consecutive_burst_info_exceed_consecutive) {
  vector<bool> packet_losses = vector<bool> {0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 1,
      1, 1, 1, 1, 1, 0, 0, 0 };
  vector<uint16_t> frame_indices = {0, 3, 7, 10, 12, 16};
  uint16_t num_frames = 5;
  uint16_t num_pkts = 16;
  auto test = LossMetricComputer(LossInfo(), 0).consecutive_burst_info(
      packet_losses, frame_indices);
  EXPECT_EQ(num_frames,test.first);
  EXPECT_EQ(num_pkts,test.second);
}

TEST(test_loss_metric_computer, test_consecutive_burst_info_after_start) {
  vector<bool> packet_losses = vector<bool> { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
      0, 1, 1, 1, 1, 1, 1, 0, 0, 0};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14, 18};
  uint16_t num_frames = 0;
  uint16_t num_pkts = 0;
  auto test = LossMetricComputer(LossInfo(), 0).consecutive_burst_info(
      packet_losses, frame_indices);
  EXPECT_EQ(num_frames,test.first);
  EXPECT_EQ(num_pkts,test.second);
}

TEST(test_loss_metric_computer, test_consecutive_burst_info_end_before_consecutive) {
  vector<bool> packet_losses = vector<bool>(21, true);
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14, 18};
  uint16_t num_frames = 7;
  uint16_t num_pkts = 21;
  auto test = LossMetricComputer(LossInfo(), 0).consecutive_burst_info(
      packet_losses, frame_indices);
  EXPECT_EQ(num_frames,test.first);
  EXPECT_EQ(num_pkts,test.second);
}

TEST(test_loss_metric_computer, test_consecutive_burst_info_through_end) {
  vector<bool> packet_losses = vector<bool> { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 0,1,0};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14, 18};
  uint16_t num_frames = 7;
  uint16_t num_pkts = 21;
  auto test = LossMetricComputer(LossInfo(), 0).consecutive_burst_info(
      packet_losses, frame_indices);
  EXPECT_EQ(num_frames,test.first);
  EXPECT_EQ(num_pkts,test.second);
}

TEST(test_loss_metric_computer, test_consecutive_burst_info_gap) {
  vector<bool> packet_losses = vector<bool> { 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1,
      1, 1, 1, 1, 1, 1, 1};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14};
  uint16_t num_frames = 2;
  uint16_t num_pkts = 5;
  auto test = LossMetricComputer(LossInfo(), 0).consecutive_burst_info(
      packet_losses, frame_indices);
  EXPECT_EQ(num_frames,test.first);
  EXPECT_EQ(num_pkts,test.second);

}

TEST(test_loss_metric_computer, test_get_positions_consecutive_losses_empty) {
  vector<bool> packet_losses;
  vector<uint16_t> frame_indices;
  uint16_t min_num_consecutive_frames_to_eval = 2;
  vector<uint16_t> positions = {};
  vector<uint16_t> lengths = {};
  auto test = LossMetricComputer(LossInfo(), 0).
      get_positions_consecutive_losses(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);
  EXPECT_EQ(positions,test.first);
  EXPECT_EQ(lengths,test.second);
}

TEST(test_loss_metric_computer, test_get_positions_consecutive_losses_no_valid_positions) {
  vector<bool> packet_losses = vector<bool> (18, false);
  vector<uint16_t> frame_indices = {0, 3, 6, 10, 13, 15};
  uint16_t min_num_consecutive_frames_to_eval = 2;
  vector<uint16_t> positions = {};
  vector<uint16_t> lengths = {};
  auto test = LossMetricComputer(LossInfo(), 0).
      get_positions_consecutive_losses(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);
  EXPECT_EQ(positions,test.first);
  EXPECT_EQ(lengths,test.second);
}

TEST(test_loss_metric_computer, test_get_positions_consecutive_losses_one_valid_position) {
  vector<bool> packet_losses = vector<bool> { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
      0, 0, 0, 0, 0, 0, 0};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14, 18};
  uint16_t min_num_consecutive_frames_to_eval = 2;
  vector<uint16_t> positions = {2};
  vector<uint16_t> lengths = {7};
  auto test = LossMetricComputer(LossInfo(), 0).
      get_positions_consecutive_losses(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);
  EXPECT_EQ(positions,test.first);
  EXPECT_EQ(lengths,test.second);
}

TEST(test_loss_metric_computer, test_get_positions_consecutive_losses_two_valid_positions) {
  vector<bool> packet_losses = vector<bool> { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
      0, 1, 1, 1, 1, 1, 1, 0, 0, 0};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14, 18};
  uint16_t min_num_consecutive_frames_to_eval = 2;
  vector<uint16_t> positions = {2, 12};
  vector<uint16_t> lengths = {7, 6};
  auto test = LossMetricComputer(LossInfo(), 0).
      get_positions_consecutive_losses(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);
  EXPECT_EQ(positions,test.first);
  EXPECT_EQ(lengths,test.second);
}

TEST(test_loss_metric_computer, test_get_positions_consecutive_losses_three_valid_position) {
  vector<bool> packet_losses = vector<bool> { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
      0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
      1, 1, 1, 1, 0, 0};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14, 18, 21, 26, 32, 35, 39};
  uint16_t min_num_consecutive_frames_to_eval = 2;
  vector<uint16_t> positions = {2, 12, 32};
  vector<uint16_t> lengths = {7, 6, 7};
  auto test = LossMetricComputer(LossInfo(), 0).
      get_positions_consecutive_losses(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);
  EXPECT_EQ(positions,test.first);
  EXPECT_EQ(lengths,test.second);
}

TEST(test_loss_metric_computer, test_get_positions_consecutive_losses_beginning_position) {
  vector<bool> packet_losses = vector<bool> { 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0,
      0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
      1, 1, 1, 1, 0, 0};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14, 18, 21, 26, 32, 35, 39};
  uint16_t min_num_consecutive_frames_to_eval = 2;
  vector<uint16_t> positions = {0, 12, 32};
  vector<uint16_t> lengths = {5, 6, 7};
  auto test = LossMetricComputer(LossInfo(), 0).
      get_positions_consecutive_losses(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);
  EXPECT_EQ(positions,test.first);
  EXPECT_EQ(lengths,test.second);
}

TEST(test_loss_metric_computer, test_get_positions_consecutive_losses_end_position) {
  vector<bool> packet_losses = vector<bool> { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
      0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
      1, 1, 1, 1};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14, 18, 21, 26, 32, 35};
  uint16_t min_num_consecutive_frames_to_eval = 2;
  vector<uint16_t> positions = {2, 12, 32};
  vector<uint16_t> lengths = {7, 6, 7};
  auto test = LossMetricComputer(LossInfo(), 0).
      get_positions_consecutive_losses(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);
  EXPECT_EQ(positions,test.first);
  EXPECT_EQ(lengths,test.second);
}

TEST(test_loss_metric_computer, test_get_positions_consecutive_losses_single_consecutive_position_considered) {
  vector<bool> packet_losses = vector<bool> { 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0,
      0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
      1, 1, 1, 1};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14, 18, 21, 26, 32, 35};
  uint16_t min_num_consecutive_frames_to_eval = 1;
  vector<uint16_t> positions = {0, 5, 12, 32};
  vector<uint16_t> lengths = {2, 4, 6, 7};
  auto test = LossMetricComputer(LossInfo(), 0).
      get_positions_consecutive_losses(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);
  EXPECT_EQ(positions,test.first);
  EXPECT_EQ(lengths,test.second);
}

TEST(test_loss_metric_computer, test_get_positions_consecutive_losses_three_consecutive_positions_considered) {
  vector<bool> packet_losses = vector<bool> { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
      0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
      1, 1, 1, 1};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14, 18, 21, 26, 32, 35};
  uint16_t min_num_consecutive_frames_to_eval = 3;
  vector<uint16_t> positions = {12};
  vector<uint16_t> lengths = {9};
  auto test = LossMetricComputer(LossInfo(), 0).
      get_positions_consecutive_losses(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);
  EXPECT_EQ(positions,test.first);
  EXPECT_EQ(lengths,test.second);
}

TEST(test_loss_metric_computer, test_get_positions_consecutive_losses_more_than_consecutive_positions_qualify_considered) {
  vector<bool> packet_losses = vector<bool> { 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
      0, 0, 0, 0};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14, 18, 21, 26, 32, 35};
  uint16_t min_num_consecutive_frames_to_eval = 2;
  vector<uint16_t> positions = {2, 26};
  vector<uint16_t> lengths = {19, 9};
  auto test = LossMetricComputer(LossInfo(), 0).
      get_positions_consecutive_losses(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);
  EXPECT_EQ(positions,test.first);
  EXPECT_EQ(lengths,test.second);

}

TEST(test_loss_metric_computer, test_compute_compute_multi_frame_loss_fraction_empty) {
  vector<bool> packet_losses;
  vector<uint16_t> frame_indices;
  uint16_t min_num_consecutive_frames_to_eval = 2;
  float test_average_loss_frac = LossMetricComputer(LossInfo(), 0).
      compute_multi_frame_loss_fraction(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);;
  float average_loss_frac = 0.0;
  EXPECT_EQ(average_loss_frac, test_average_loss_frac);
}

TEST(test_loss_metric_computer, test_compute_compute_multi_frame_loss_fraction_no_valid_positions) {
  vector<bool> packet_losses = vector<bool> (18, false);
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14};
  uint16_t min_num_consecutive_frames_to_eval = 2;
  float test_average_loss_frac = LossMetricComputer(LossInfo(), 0).
      compute_multi_frame_loss_fraction(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);;
  float average_loss_frac = 0.0;
  EXPECT_EQ(average_loss_frac, test_average_loss_frac);
}

TEST(test_loss_metric_computer, test_compute_compute_multi_frame_loss_fraction_one_valid_position) {
  vector<bool> packet_losses = vector<bool> { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
      0, 0, 0, 0, 0, 0, 0};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14};
  uint16_t min_num_consecutive_frames_to_eval = 2;
  float test_average_loss_frac = LossMetricComputer(LossInfo(), 0).
      compute_multi_frame_loss_fraction(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);;
  float average_loss_frac = 1.0;
  EXPECT_EQ(average_loss_frac, test_average_loss_frac);
}

TEST(test_loss_metric_computer, test_compute_compute_multi_frame_loss_fraction_two_valid_positions) {
  vector<bool> packet_losses = vector<bool> { 0, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0,
      0, 1, 1, 1, 1, 1, 1, 0, 0, 0};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14, 18};
  uint16_t min_num_consecutive_frames_to_eval = 2;
  vector<uint16_t> positions = {2, 12};
  vector<uint16_t> lengths = {7, 6};
  float test_average_loss_frac = LossMetricComputer(LossInfo(), 0).
      compute_multi_frame_loss_fraction(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);;
  float average_loss_frac = 1.0;
  EXPECT_EQ(average_loss_frac, test_average_loss_frac);
}

TEST(test_loss_metric_computer, test_compute_compute_multi_frame_loss_fraction_three_valid_position) {
  vector<bool> packet_losses = vector<bool> { 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0,
      0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1,
      1, 1, 1, 1, 0, 0};
  vector<uint16_t> frame_indices = {0, 2, 5, 9, 12, 14, 18, 21, 26, 32, 35, 39};
  uint16_t min_num_consecutive_frames_to_eval = 2;
  float test_average_loss_frac = LossMetricComputer(LossInfo(), 0).
      compute_multi_frame_loss_fraction(packet_losses, frame_indices,
      min_num_consecutive_frames_to_eval);;
  float average_loss_frac = 1.0/3*(6.0/7+.5+1.0);
  EXPECT_NEAR(average_loss_frac, test_average_loss_frac, err);
}

TEST(test_loss_metric_computer, test_no_keys)
{
  LossInfo lossInfo;
  float mean_length = 0.0;
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto stats = lossMetricComputer.get_bursts_and_guardspaces(
        lossInfo.packet_losses);
  auto test_mean_length = lossMetricComputer.compute_mean_length(stats.first);
  EXPECT_NEAR(mean_length,test_mean_length, err);
}

TEST(test_loss_metric_computer, test_single_key)
{
  for (uint16_t i = 1; i < 10; i++) {
    for (uint16_t j = 3; j < 8; j ++) {
      map<uint16_t, uint16_t> stats = {{i,j}};
      float mean_length = float(i);
      LossMetricComputer lossMetricComputer(LossInfo(), 0);
      float test_mean_length = lossMetricComputer.compute_mean_length(stats);
      EXPECT_NEAR(mean_length,test_mean_length, err);
    }
  }
}

TEST(test_loss_metric_computer, test_multiple_keys)
{
  for (uint16_t i1 = 1; i1 < 10; i1++) {
    for (uint16_t i2 = 3; i2 < 8; i2++) {
      for (uint16_t j1 = 1; j1 < 10; j1++) {
        if (j1 == i1) { continue; }
        for (uint16_t j2 = 3; j2 < 8; j2++) {
          map<uint16_t, uint16_t> stats = { {i1, i2}, {j1, j2} };
          float mean_length = float(i1*i2 +j1*j2)/(i2+j2);
          LossMetricComputer lossMetricComputer(LossInfo(), 0);
          float test_mean_length = lossMetricComputer.compute_mean_length(
              stats);
          EXPECT_NEAR(mean_length,test_mean_length, err);
        }
      }
    }
  }
}

TEST(test_loss_metric_computer, test_empty_pkts)
{
  vector<bool> packet_losses;
  float loss_fraction = 0.0;
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  float test_loss_fraction = lossMetricComputer.compute_loss_fraction(
      packet_losses);
  EXPECT_EQ(loss_fraction,test_loss_fraction);
}

TEST(test_loss_metric_computer, test_all)
{
  vector<bool> packet_losses(4, true);
  float loss_fraction = 1.0;
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  float test_loss_fraction = lossMetricComputer.compute_loss_fraction(
      packet_losses);
  EXPECT_EQ(loss_fraction,test_loss_fraction);
}

TEST(test_loss_metric_computer, test_none)
{
  vector<bool> packet_losses(4, false);
  float loss_fraction = 0.0;
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  float test_loss_fraction = lossMetricComputer.compute_loss_fraction(
      packet_losses);
  EXPECT_EQ(loss_fraction,test_loss_fraction);
}

TEST(test_loss_metric_computer, test_multiple_instances)
{
  vector<bool> packet_losses = vector<bool> {false, false, false, false, true, true, false,
      true, true, true };
  float loss_fraction = 0.5;
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  float test_loss_fraction = lossMetricComputer.compute_loss_fraction(
      packet_losses);
  EXPECT_EQ(loss_fraction,test_loss_fraction);

}

TEST(test_loss_metric_computer, test_empty_guardspaces)
{
  map<uint16_t, uint16_t> guardspaces;
  uint16_t multi_frame_minimal_guardspace = 3;
  float val = 0;
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test_val =
      lossMetricComputer.compute_multi_frame_insufficient_guardspace(
      guardspaces, multi_frame_minimal_guardspace);
  EXPECT_EQ(val,test_val);
}

TEST(test_loss_metric_computer, test_no_keys_less_than_min)
{
  map<uint16_t, uint16_t> guardspaces = {{1, 2}, {2, 3}};
  float multi_frame_minimal_guardspace = 3;
  float val = 1.0;
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test_val =
      lossMetricComputer.compute_multi_frame_insufficient_guardspace(
      guardspaces, multi_frame_minimal_guardspace);
  EXPECT_EQ(val,test_val);
}

TEST(test_loss_metric_computer, test_all_keys_more_than_min)
{
  map<uint16_t, uint16_t> guardspaces = {{3, 2}, {4, 3}};
  float multi_frame_minimal_guardspace = 3;
  float val = 0.0;
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test_val =
      lossMetricComputer.compute_multi_frame_insufficient_guardspace(
      guardspaces, multi_frame_minimal_guardspace);
  EXPECT_EQ(val,test_val);
}

TEST(test_loss_metric_computer, test_some_less_some_more)
{
  map<uint16_t, uint16_t> guardspaces = {{1, 5}, {2, 7}, {3, 1}, {4, 3}};
  float multi_frame_minimal_guardspace = 3;
  float val = .75;
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test_val =
      lossMetricComputer.compute_multi_frame_insufficient_guardspace(
      guardspaces, multi_frame_minimal_guardspace);
  EXPECT_EQ(val,test_val);
}

TEST(test_loss_metric_computer, test_min_2)
{
  map<uint16_t, uint16_t> guardspaces = {{1, 4}, {2, 8}, {3, 1}, {4, 3}};
  float multi_frame_minimal_guardspace = 2;
  float val = .25;
  LossMetricComputer lossMetricComputer(LossInfo(), 0);
  auto test_val =
      lossMetricComputer.compute_multi_frame_insufficient_guardspace(
      guardspaces, multi_frame_minimal_guardspace);
  EXPECT_EQ(val,test_val);
}
const LossInfo testLossInfo = { {0, 2, 5, 8, 28, 31, 34, 37, 57, 58, 59, 60,
    61, 62}, {1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, {1, 0, 1, 0, 1, 1, 1, 0,
    0, 0, 1, 0, 0, 0} };

TEST(test_loss_metric_computer, test_eq_optional_args)
{
  LossMetricComputer indicators(testLossInfo, 1, 3, 3, 2, 2);
  LossMetricComputer indicators2(testLossInfo, 1, 3, 3, 2, 2);
  indicators.compute_loss_metrics();
  indicators2.compute_loss_metrics();
  EXPECT_TRUE(indicators.compute_loss_metrics() ==
      indicators2.compute_loss_metrics());
  LossMetricComputer indicators3(testLossInfo, 1, 4, 3, 2, 2);
  EXPECT_FALSE(indicators.compute_loss_metrics() ==
      indicators3.compute_loss_metrics());
  LossMetricComputer indicators4(testLossInfo, 1, 3, 4, 2, 2);
  EXPECT_FALSE(indicators.compute_loss_metrics() ==
      indicators4.compute_loss_metrics());
  LossMetricComputer indicators5(testLossInfo, 1, 3, 3, 1, 2);
  EXPECT_FALSE(indicators.compute_loss_metrics() ==
      indicators5.compute_loss_metrics());
  LossMetricComputer indicators6(testLossInfo, 1, 3, 3, 2, 1);
  EXPECT_FALSE(indicators.compute_loss_metrics() ==
      indicators6.compute_loss_metrics());
}

TEST(test_loss_metric_computer, test_init_and_get_indicators)
{
  LossMetricComputer indicators(testLossInfo, 1, 3, 3, 2, 2);
  auto test_vals = indicators.compute_loss_metrics();
  EXPECT_NEAR(test_vals.burst_density,  11.0 / 14.0, err);
  EXPECT_NEAR(test_vals.frame_burst_density, 5.0 / 7, err);
  EXPECT_NEAR(test_vals.gap_density, 1.0 / 49.0, err);
  EXPECT_NEAR(test_vals.frame_gap_density, 1.0 / 7.0, err);
  EXPECT_NEAR(test_vals.mean_burst_length,
      (2.0 + 3.0 + 1 * 3 + 2 * 2) / (7.0), err);
  EXPECT_NEAR(test_vals.mean_frame_burst_length, 6.0 / 4.0, err);
  EXPECT_NEAR(test_vals.loss_fraction, 12.0 / 63.0, err);
  EXPECT_NEAR(test_vals.frame_loss_fraction, 6.0 / 14, err);
  EXPECT_NEAR(test_vals.multi_frame_loss_fraction, 2.0 / 3, err);
  EXPECT_NEAR(test_vals.mean_guardspace_length, 51.0 / 7, err);
  EXPECT_NEAR(test_vals.mean_frame_guardspace_length, 2.0, err);
  EXPECT_NEAR(test_vals.multi_frame_insufficient_guardspace, 0.5, err);
  EXPECT_NEAR(test_vals.qr, 1, err);
}

TEST(test_loss_metric_computer, test_get_window_metrics)
{
  LossInfo testLossInfo = { {0, 2, 5, 8, 28, 31, 34, 37, 57, 58, 59, 60,
      61, 62}, {1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0,
      0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0}, {1, 0, 1, 0, 1, 1,
      1, 0, 0, 0, 1, 0, 0, 0} };
  LossMetricComputer computer(testLossInfo, 1, 3, 3, 2, 2);
  auto test_vals = computer.get_window_metrics(testLossInfo);
  tuple<float,float,float> vals { 63, 12, 3 };
  EXPECT_EQ(test_vals, vals);
}

int main(int argc, char * argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
