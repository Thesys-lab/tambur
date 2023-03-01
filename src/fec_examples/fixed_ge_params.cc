#include <iostream>
#include <fstream>
#include <string>
#include <deque>
#include <filesystem>

#include "parse_cfg.hh"
#include "json.hpp"
#include "loss_info.hh"
#include "loss_metrics.hh"
#include "loss_computer.hh"
#include "loss_metric_computer.hh"

using namespace std;
using json = nlohmann::json;

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        cerr << "Usage: " << argv[0] << " config trace_file" << endl;
        return -1;
    }

    Config::parse(argv[1]);

    std::ifstream json_trace_file(argv[2]);
    json trace;
    json_trace_file >> trace;

    std::vector<bool> pkt_drop;
    std::vector<int> frame_numbers;
    
    pkt_drop = trace["trace"].get<std::vector<bool>>();
    frame_numbers = trace["frame_numbers"].get<std::vector<int>>();

    LossInfo loss_info;
    LossMetrics loss_metric;
        
    // indices
    loss_info.indices.push_back(0);
    auto prev_frame_num = frame_numbers[0];
    for (size_t i = 0; i < frame_numbers.size(); ++i) {
        if (frame_numbers.at(i) != prev_frame_num) {
            loss_info.indices.push_back(loss_info.indices.back()+1);
        }
        prev_frame_num = frame_numbers.at(i);
    }

    // packet_losses
    loss_info.packet_losses.resize(pkt_drop.size());  
    for (size_t i = 0; i < pkt_drop.size(); ++i) {
        if (pkt_drop.at(i)) {
            loss_info.packet_losses[i] = true;
        } else {
            loss_info.packet_losses[i] = false;
        }
    }

    // frame_losses
    loss_info.frame_losses.resize(loss_info.indices.size());
    for (size_t i = 0; i < loss_info.frame_losses.size(); ++i) {
        loss_info.frame_losses[i] = false;
    }

    for (size_t i = 0; i < pkt_drop.size(); ++i) {
        if (pkt_drop.at(i)) {
            loss_info.frame_losses[frame_numbers[i]] = true;
        }
    }

    LossMetricComputer loss_metric_computer(loss_info, 1, Config::ge_gMin_pkt);
    loss_metric = loss_metric_computer.compute_loss_metrics();
    auto t_bg = 1.0 / loss_metric.mean_frame_burst_length;
    auto t_gb = 1.0 / loss_metric.mean_frame_guardspace_length;
    auto l_b = loss_metric_computer.compute_multi_frame_loss_fraction(loss_info.packet_losses,
            loss_info.indices, 2);
    auto l_g = loss_metric.loss_fraction * (t_bg + t_gb) / t_bg 
        - l_b * t_gb / t_bg;


    std::cout << t_gb << " " << t_bg << " " << l_b << " " << l_g << std::endl;

    return 0;
}