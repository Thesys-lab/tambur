#ifndef STRATEGY_HH
#define STRATEGY_HH

#include <unordered_map>
#include <fstream>

#include "json.hpp"
#include "loss_info.hh"
#include "loss_metric_computer.hh"

using json = nlohmann::json;
using namespace std;

class GEStrategy
{
public:
    GEStrategy(uint16_t tau, const char* trace_file, uint16_t ge_gMin_pkt=1) : tau_(tau)
    {
        std::ifstream json_trace_file(trace_file);
        json_trace_file >> trace_;

        std::vector<bool> pkt_drop;
        std::vector<int> frame_numbers;
        
        pkt_drop = trace_["trace"].get<std::vector<bool>>();
        frame_numbers = trace_["frame_numbers"].get<std::vector<int>>();

        // indices
        loss_info_.indices.push_back(0);
        auto prev_frame_num = frame_numbers[0];
        for (size_t i = 0; i < frame_numbers.size(); ++i) {
            if (frame_numbers.at(i) != prev_frame_num) {
                loss_info_.indices.push_back(loss_info_.indices.back()+1);
            }
            prev_frame_num = frame_numbers.at(i);
        }

        // packet_losses
        loss_info_.packet_losses.resize(pkt_drop.size());  
        for (size_t i = 0; i < pkt_drop.size(); ++i) {
            if (pkt_drop.at(i)) {
                loss_info_.packet_losses[i] = true;
            } else {
                loss_info_.packet_losses[i] = false;
            }
        }

        // frame_losses
        loss_info_.frame_losses.resize(loss_info_.indices.size());
        for (size_t i = 0; i < loss_info_.frame_losses.size(); ++i) {
            loss_info_.frame_losses[i] = false;
        }

        for (size_t i = 0; i < pkt_drop.size(); ++i) {
            if (pkt_drop.at(i)) {
                loss_info_.frame_losses[frame_numbers[i]] = true;
            }
        }


        LossMetricComputer loss_metric_computer(loss_info_, 1, ge_gMin_pkt);
        loss_metric_ = loss_metric_computer.compute_loss_metrics();
        ge_gMin_pkt_ = ge_gMin_pkt;
    }

    virtual void setLossGoodState() = 0;
    virtual void setLossBadState() = 0;
    virtual void setTransitionGoodBad() = 0;
    virtual void setTransitionBadGood() = 0;

    double probGoodState() { return t_bg_/(t_bg_ + t_gb_); }
    double probBadState()  { return t_gb_/(t_bg_ + t_gb_); }

    double l_g_;  // loss probability in good state
    double l_b_;  // loss probability in bad state
    double t_gb_; // probability of transition from good to bad state
    double t_bg_; // probability of transition from bad to good state

protected:
    unordered_map<int, double> distribution_packets_tau_frames()
    {
        auto frame_numbers = trace_["frame_numbers"].get<std::vector<int>>();
        vector<int> num_pkts;
        int last_frame = -1;
        for (auto fn : frame_numbers)
        {
            if (fn != last_frame) {
                num_pkts.push_back(1);
            } else {
                num_pkts.back() += 1;
            }
            last_frame = fn;
        }

        unordered_map<int, double> prob;
        size_t cur_pkts_tau = 0;
        for (uint16_t i = 0; i < tau_; ++i)
        {
            cur_pkts_tau += num_pkts[i];
        }
        prob[cur_pkts_tau]+=1;

        for (size_t i = tau_; i < num_pkts.size(); ++i)
        {
            cur_pkts_tau -= num_pkts[i-tau_];
            cur_pkts_tau += num_pkts[i];
            prob[cur_pkts_tau]+=1;
        }

        for (auto& it : prob)
        {
            it.second /= (num_pkts.size()-tau_+1);
        }

        return prob;
    }

    double prob_guard_space_less_tau()
    {
        auto loss_metric_computer = LossMetricComputer(loss_info_, 1);
        auto guard_spaces = loss_metric_computer.get_bursts_and_guardspaces(loss_info_.frame_losses).second;

        double total_gs = 0;
        double gs_less_tau = 0;
        for (auto const& it : guard_spaces) {
            if (it.first < tau_) {
                gs_less_tau += it.second;
            }
            total_gs += it.second;
        }
        return gs_less_tau/total_gs;
    }

    double get_tgb_guard_space_match() {
        auto D = distribution_packets_tau_frames();
        auto ptau = 1-prob_guard_space_less_tau();

        assert(ptau <= 1.0);

        double lo = 0, hi = PRECISION, mid;
        double min_diff = 1.0, ans = 0.0;

        while (lo <= hi) {
            mid = lo + (hi-lo)/2;
            double p_gb = mid/PRECISION;
            auto pgsl = prob_guard_space_long(D, p_gb);
            assert(pgsl <= 1.0);
            if (abs(pgsl-ptau) < min_diff)
            {
                ans = p_gb;
                min_diff = abs(pgsl-ptau);
            }

            if (pgsl > ptau) {
                lo =  mid+1;
            } else if (pgsl < ptau) {
                hi = mid-1;
            } else {
                break;
            }
        }
        return ans;
    }

private:
    double prob_guard_space_long(unordered_map<int, double>& D, double p_gb) {
        double sum = 0;
        for (auto const it : D) {
            sum += it.second*pow(1-p_gb, it.first);
        }
        return sum;
    }

protected:
    static constexpr int PRECISION = 1000000;
    uint16_t tau_;
    LossInfo loss_info_;
    LossMetrics loss_metric_;
    uint16_t ge_gMin_pkt_;
    json trace_;
};

#endif