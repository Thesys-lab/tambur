#include <cmath>

#include "strategy.hh"

#ifndef STRATEGY_DENSITY_MATCH_HH
#define STRATEGY_DENSITY_MATCH_HH

class DensityMatch : public GEStrategy
{
public:
    DensityMatch(uint16_t tau, const char* trace_file, uint16_t ge_gMin_pkt) : GEStrategy(tau, trace_file, ge_gMin_pkt) {
        setLossGoodState();
        setLossBadState();
        setTransitionBadGood();
        setTransitionGoodBad();
        

        assert(l_g_  >= 0 && l_g_  <= 1.0);
        assert(l_b_  >= 0 && l_b_  <= 1.0);
        assert(t_gb_ >= 0 && t_gb_ <= 1.0);
        assert(t_bg_ >= 0 && t_bg_ <= 1.0);
    }
private:
    void setLossGoodState() override {
        l_g_ = loss_metric_.gap_density;
    }

    void setLossBadState() override {
        l_b_ = loss_metric_.burst_density;
    }

    void setTransitionGoodBad() override {
        if (l_b_ <= loss_metric_.loss_fraction) {
                t_gb_ = 0.0;
        }
        else {
                t_gb_ = t_bg_*(loss_metric_.loss_fraction - l_g_) / (l_b_ - loss_metric_.loss_fraction);
        }
    }

    void setTransitionBadGood() override {
        LossMetricComputer lmc(loss_info_, 1, ge_gMin_pkt_);
        auto burstIndices = lmc.getBurstIndices(loss_info_.packet_losses, ge_gMin_pkt_);
        float mean = 0;
        for (auto bI : burstIndices)
        {
            mean += (bI.second - bI.first + 1);
        }
        mean /= burstIndices.size();
        
        t_bg_ = 1.0/mean;
    }
};

#endif
