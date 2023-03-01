#include <cmath>

#include "strategy.hh"

#ifndef FRAME_MATCH_HH
#define FRAME_MATCH_HH

class FrameMatch : public GEStrategy
{
public:
    FrameMatch(uint16_t tau, const char* trace_file, uint16_t ge_gMin_pkt=2) : GEStrategy(tau, trace_file, ge_gMin_pkt) {
        setLossBadState();
        setTransitionBadGood();
        setTransitionGoodBad();
        setLossGoodState();
        assert(l_g_  >= 0 && l_g_  <= 1.0);
        assert(l_b_  >= 0 && l_b_  <= 1.0);
        assert(t_gb_ >= 0 && t_gb_ <= 1.0);
        assert(t_bg_ >= 0 && t_bg_ <= 1.0);
    }
private:
    void setLossGoodState() override {
      if (loss_metric_.mean_frame_burst_length == 0.0 ||
        loss_metric_.mean_frame_guardspace_length == 0.0) {
          l_g_ = loss_metric_.loss_fraction;
      }  else {
        l_g_ = loss_metric_.loss_fraction* (t_bg_ + t_gb_) / (t_bg_)
            - l_b_ * t_gb_ / t_bg_;
        l_g_ = max(0.0, l_g_);
        l_g_ = min(1.0, l_g_);
      }
    }

    void setLossBadState() override {
      if (loss_metric_.mean_frame_burst_length == 0.0 ||
      loss_metric_.mean_frame_guardspace_length == 0.0) {
        l_b_ = loss_metric_.loss_fraction;
      } else {
        LossMetricComputer lmc(loss_info_, 1, ge_gMin_pkt_);
        l_b_ = lmc.compute_multi_frame_loss_fraction(loss_info_.packet_losses,
        loss_info_.indices, 2);
      }
    }

    void setTransitionGoodBad() override {
      if (loss_metric_.mean_frame_burst_length == 0.0 ||
        loss_metric_.mean_frame_guardspace_length == 0.0) {
        t_gb_ = 0.0;
      } else {
        t_gb_ = 1.0 / loss_metric_.mean_frame_guardspace_length;
      }
    }

    void setTransitionBadGood() override {
      if (loss_metric_.mean_frame_burst_length == 0.0 ||
        loss_metric_.mean_frame_guardspace_length == 0.0) {
        t_bg_ = 0.0;
      } else {
        t_bg_ = 1.0 / loss_metric_.mean_frame_burst_length;
      }
    }
};

#endif
