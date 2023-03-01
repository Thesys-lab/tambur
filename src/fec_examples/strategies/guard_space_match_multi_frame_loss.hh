#include <cmath>

#include "strategy.hh"

#ifndef STRATEGY_GUARD_SPACE_MATCH_MULTI_FRAME_LOSS_HH
#define STRATEGY_GUARD_SPACE_MATCH_MULTI_FRAME_LOSS_HH

class GuardSpaceMatchMultiFrameLoss : public GEStrategy
{
public:
    GuardSpaceMatchMultiFrameLoss(uint16_t tau, const char* trace_file) : GEStrategy(tau, trace_file) {
        setLossGoodState();
        setLossBadState();
        setTransitionGoodBad();
        setTransitionBadGood();

        assert(l_g_  >= 0 && l_g_  <= 1.0);
        assert(l_b_  >= 0 && l_b_  <= 1.0);
        assert(t_gb_ >= 0 && t_gb_ <= 1.0);
        assert(t_bg_ >= 0 && t_bg_ <= 1.0);
    }
private:
    void setLossGoodState() override {
        l_g_ = 0;
    }

    void setLossBadState() override {
        l_b_ = loss_metric_.multi_frame_loss_fraction;
    }

    void setTransitionGoodBad() override {
        t_gb_ = get_tgb_guard_space_match();
    }

    void setTransitionBadGood() override {
        t_bg_ = t_gb_ * (loss_metric_.multi_frame_loss_fraction - loss_metric_.loss_fraction) / (loss_metric_.loss_fraction);
    }
};

#endif