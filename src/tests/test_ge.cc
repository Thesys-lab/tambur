#include "gtest/gtest.h"

#include "ge_channel.hh"

using namespace std;

TEST(test_ge, test_good_bad_transition_0_prob)
{
  GEChannel gE(0,0,0,0,0);
  for (uint64_t j = 0; j < 10000; j++) {
    gE.is_received();
    EXPECT_FALSE(gE.get_state());
  }
}

TEST(test_ge, test_good_bad_transition_and_back_1_prob)
{
  float p_good_to_bad = 1.0;
  float p_bad_to_good = 1.0;
  float p_loss_good = 0.0;
  float p_loss_bad = 0.0;
  int seed = rand() % 100;
  GEChannel gE(p_good_to_bad,p_bad_to_good,p_loss_good,p_loss_bad,seed);
  for (uint64_t j = 0; j < 10000; j++) {
    EXPECT_EQ(gE.get_state(), j % 2);
    gE.transition();
  }
}

TEST(test_ge, test_good_bad_transition_half_prob)
{
  float p_good_to_bad = 0.5;
  float p_bad_to_good = 1.0;
  float p_loss_good = 0.0;
  float p_loss_bad = 0.0;
  int seed = rand() % 100;
  uint64_t num_trials = 1000000;
  GEChannel gE(p_good_to_bad,p_bad_to_good,p_loss_good,p_loss_bad,seed);
  float count = 0.0;
  for (uint64_t j = 0; j < num_trials; j++) {
    gE.transition();
    gE.transition();
    count += gE.get_state();
  }
  EXPECT_GE(count, .25*num_trials);
  EXPECT_LE(count, .4*num_trials);
}

TEST(test_ge, test_bad_good_transition_0_prob)
{
  float p_good_to_bad = 1.0;
  float p_bad_to_good = 0.0;
  float p_loss_good = 0.0;
  float p_loss_bad = 0.0;
  int seed = rand() % 100;
  uint64_t num_trials = 10000;
  GEChannel gE(p_good_to_bad,p_bad_to_good,p_loss_good,p_loss_bad,seed);
  for (uint64_t j = 0; j < num_trials; j++) {
    gE.transition();
    EXPECT_TRUE(gE.get_state());
  }
}

TEST(test_ge, test_bad_good_transition_half_prob)
{
  float p_good_to_bad = 1;
  float p_bad_to_good = .5;
  float p_loss_good = 0;
  float p_loss_bad = 0;
  int seed = rand() % 100;
  uint64_t num_trials = 10000;
  GEChannel gE(p_good_to_bad,p_bad_to_good,p_loss_good,p_loss_bad,seed);
  float count = 0.0;
  for (uint64_t j = 0; j < num_trials; j++) {
    gE.transition();
    count += !gE.get_state();
  }
  EXPECT_GE(count, .25*num_trials);
  EXPECT_LE(count, .4*num_trials);
}

TEST(test_ge, test_loss_bad_0_prob)
{
  float p_good_to_bad = 1.0;
  float p_bad_to_good = 0.0;
  float p_loss_good = 1.0;
  float p_loss_bad = 0.0;
  int seed = rand() % 100;
  uint64_t num_trials = 10000;
  GEChannel gE(p_good_to_bad,p_bad_to_good,p_loss_good,p_loss_bad,seed);
  for (uint64_t j = 0; j < num_trials; j++) {
    gE.transition();
    EXPECT_TRUE(gE.is_received());
  }
}

TEST(test_ge, test_loss_bad_1_prob)
{
  float p_good_to_bad = 1.0;
  float p_bad_to_good = 0.0;
  float p_loss_good = 0.0;
  float p_loss_bad = 1.0;
  int seed = rand() % 100;
  uint64_t num_trials = 10000;
  GEChannel gE(p_good_to_bad,p_bad_to_good,p_loss_good,p_loss_bad,seed);
  for (uint64_t j = 0; j < num_trials; j++) {
    gE.transition();
    EXPECT_FALSE(gE.is_received());
  }

}

TEST(test_ge, test_loss_bad_half_prob)
{
  float p_good_to_bad = 1.0;
  float p_bad_to_good = 0.0;
  float p_loss_good = 0.0;
  float p_loss_bad = 0.25;
  int seed = rand() % 100;
  uint64_t num_trials = 10000;
  GEChannel gE(p_good_to_bad,p_bad_to_good,p_loss_good,p_loss_bad,seed);
  float count = 0;
  for (uint64_t j = 0; j < num_trials; j++) {
    gE.transition();
    count += gE.is_received();
  }
  EXPECT_GE(count, num_trials * .7);
  EXPECT_LE(count, num_trials * .8);
}

TEST(test_ge, test_loss_good_0_prob)
{
  float p_good_to_bad = 0.0;
  float p_bad_to_good = 1.0;
  float p_loss_good = 0.0;
  float p_loss_bad = 1.0;
  int seed = rand() % 100;
  uint64_t num_trials = 10000;
  GEChannel gE(p_good_to_bad,p_bad_to_good,p_loss_good,p_loss_bad,seed);
  for (uint64_t j = 0; j < num_trials; j++) {
    gE.transition();
    EXPECT_TRUE(gE.is_received());
  }
}

TEST(test_ge, test_loss_good_1_prob)
{
  float p_good_to_bad = 0.0;
  float p_bad_to_good = 1.0;
  float p_loss_good = 1.0;
  float p_loss_bad = 0.0;
  int seed = rand() % 100;
  uint64_t num_trials = 10000;
  GEChannel gE(p_good_to_bad,p_bad_to_good,p_loss_good,p_loss_bad,seed);
  for (uint64_t j = 0; j < num_trials; j++) {
    gE.transition();
    EXPECT_FALSE(gE.is_received());
  }
}

TEST(test_ge, test_loss_good_half_prob)
{
  float p_good_to_bad = 0.0;
  float p_bad_to_good = 1.0;
  float p_loss_good = 0.0;
  float p_loss_bad = 0.5;
  int seed = rand() % 100;
  uint64_t num_trials = 10000;
  GEChannel gE(p_good_to_bad,p_bad_to_good,p_loss_good,p_loss_bad,seed);
  for (uint64_t j = 0; j < num_trials; j++) {
    gE.transition();
    EXPECT_TRUE(gE.is_received());
  }
}

TEST(test_ge, test_seed)
{
  float p_good_to_bad = .25;
  float p_bad_to_good = .7;
  float p_loss_good = 0.34;
  float p_loss_bad = .65;
  int seed = rand() % 100;
  uint64_t num_trials = 10000;
  GEChannel gE(p_good_to_bad,p_bad_to_good,p_loss_good,p_loss_bad,seed);
  vector<bool> first_states;
  for (uint64_t j = 0; j < num_trials; j++) {
    gE.is_received();
    gE.transition();
    first_states.push_back(gE.get_state());
  }
  GEChannel gE2(p_good_to_bad,p_bad_to_good,p_loss_good,p_loss_bad,seed);
  vector<bool> second_states;
  for (uint64_t j = 0; j < num_trials; j++) {
    gE2.is_received();
    gE2.transition();
    second_states.push_back(gE2.get_state());
  }
  for (uint64_t j = 0; j < num_trials; j++) {
    EXPECT_EQ(first_states.at(j), second_states.at(j));
  }
}

int main(int argc, char * argv[])
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
