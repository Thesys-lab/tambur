#include <iostream>
#include <fstream>
#include <string>
#include <deque>

#include "parse_cfg.hh"
#include "json.hpp"
#include "strategies/strategy.hh"
#include "strategies/guard_space_match_fixed_loss.hh"
#include "strategies/burst_time_match.hh"
#include "strategies/guard_space_match_multi_frame_loss.hh"
#include "strategies/burst_length_match_fixed_loss.hh"
#include "strategies/burst_length_match_multi_frame_loss.hh"
#include "strategies/density_match.hh"

using namespace std;
using json = nlohmann::json;


int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " config trace" << endl;
        return -1;
    }

    Config::parse(argv[1]);

    Strategy* strategy;


    if (Config::strategy_name == "burst_time_match") {
      strategy = new BurstTimeMatch(Config::tau, argv[2], 30);
      std::cout << strategy->t_gb_ << " " << strategy->t_bg_ ;
      return 0;
    } else if (Config::strategy_name == "guard_space_match_fixed_loss") {
        strategy = new GuardSpaceMatchFixedLoss(Config::tau, argv[2]);
    } else if (Config::strategy_name == "guard_space_match_multi_frame_loss") {
        strategy = new GuardSpaceMatchMultiFrameLoss(Config::tau, argv[2]);
    } else if (Config::strategy_name == "guard_space_match_multi_frame_loss") {
        strategy = new BurstLengthMatchFixedLoss(Config::tau, argv[2]);
    } else if (Config::strategy_name == "guard_space_match_multi_frame_loss") {
        strategy = new BurstLengthMatchMultiFrameLoss(Config::tau, argv[2]);
    } else if (Config::strategy_name == "density_match") {
        strategy = new DensityMatch(Config::tau, argv[2], Config::ge_gMin_pkt);
    } else {
        std::cerr << "Invalid Strategy\n";
        return -1;
    }


    std::cout << strategy->t_gb_ << " " << strategy->t_bg_ << " " << strategy->l_g_ << " " << strategy->l_b_;

    return 0;
}