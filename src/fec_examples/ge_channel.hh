#ifndef GE_CHANNEL_HH
#define GE_CHANNEL_HH
#include <random>
#include <optional>

class GEChannel
{
public:
  GEChannel(float p_good_to_bad, float p_bad_to_good, float p_loss_good,
      float p_loss_bad, int seed) : p_good_to_bad_(p_good_to_bad), p_bad_to_good_(
      p_bad_to_good), p_loss_good_(p_loss_good), p_loss_bad_(p_loss_bad),
      bad_state_(false)
  {
    generator_.seed(seed);
    distribution_ = std::uniform_real_distribution<float>(0.0, 1.0);
  }

  bool is_received();

  void transition();

  bool get_state() { return bad_state_;}

private:
  float p_good_to_bad_;
  float p_bad_to_good_;
  float p_loss_good_;
  float p_loss_bad_;
  bool bad_state_;
  std::default_random_engine generator_;
  std::uniform_real_distribution<float> distribution_;
};

#endif /* GE_CHANNEL_HH */
