#include <cmath>

#include "strategy.hh"

#ifndef STRATEGY_BURST_LENGTH_MATCH_FIXED_LOSS_HH
#define STRATEGY_BURST_LENGTH_MATCH_FIXED_LOSS_HH

class BurstLengthMatchFixedLoss : public GEStrategy
{
public:
    BurstLengthMatchFixedLoss(uint16_t tau, const char* trace_file) : GEStrategy(tau, trace_file) {
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
        l_b_ = 1;
    }

    void setTransitionGoodBad() override {
        t_gb_ = 1.0/loss_metric_.mean_guardspace_length;
    }

    void setTransitionBadGood() override {
        t_bg_ = 1.0/loss_metric_.mean_burst_length;
    }
};

#endif