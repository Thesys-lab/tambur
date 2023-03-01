#include <iostream>
#include <fstream>
#include <string>
#include <deque>

#include "parse_cfg.hh"
#include "json.hpp"
#include "strategies/strategy.hh"
#include "strategies/guard_space_match_fixed_loss.hh"
#include "strategies/guard_space_match_multi_frame_loss.hh"
#include "strategies/burst_length_match_fixed_loss.hh"
#include "strategies/burst_length_match_multi_frame_loss.hh"
#include "strategies/density_match.hh"
#include "strategies/frame_match.hh"

using namespace std;
using json = nlohmann::json;

int main(int argc, char *argv[])
{
    if (argc != 4 and argc != 3)
    {
        cerr << "Usage: " << argv[0] << " config trace [output_file]" << endl;
        return -1;
    }

    Config::parse(argv[1]);

    GEStrategy *strategy;
    if (Config::strategy_name == "frame_match")
    {
        strategy = new FrameMatch(Config::tau, argv[2]);
    }
    else if (Config::strategy_name == "guard_space_match_fixed_loss")
    {
        strategy = new GuardSpaceMatchFixedLoss(Config::tau, argv[2]);
    }
    else if (Config::strategy_name == "guard_space_match_multi_frame_loss")
    {
        strategy = new GuardSpaceMatchMultiFrameLoss(Config::tau, argv[2]);
    }
    else if (Config::strategy_name == "guard_space_match_multi_frame_loss")
    {
        strategy = new BurstLengthMatchFixedLoss(Config::tau, argv[2]);
    }
    else if (Config::strategy_name == "guard_space_match_multi_frame_loss")
    {
        strategy = new BurstLengthMatchMultiFrameLoss(Config::tau, argv[2]);
    }
    else if (Config::strategy_name == "density_match")
    {
        strategy = new DensityMatch(Config::tau, argv[2], Config::ge_gMin_pkt);
    }
    else
    {
        std::cerr << "Invalid Strategy\n";
        return -1;
    }

    if (argc == 4) {
        std::ofstream outfile;
        outfile.open(argv[3], std::ios_base::app);       
        outfile << to_string(strategy->t_gb_) + "," +
                to_string(strategy->t_bg_) + "," + to_string(strategy->l_g_) + ","
                + to_string(strategy->l_b_) + "\n";
    }

    std::cout << strategy->t_gb_ << " " << strategy->t_bg_ << " " << strategy->l_g_ << " " << strategy->l_b_ ;

    return 0;
}