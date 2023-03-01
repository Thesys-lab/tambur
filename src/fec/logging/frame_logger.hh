#ifndef FRAME_LOGGER_HH
#define FRAME_LOGGER_HH

#include <string>
#include <stack>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>
#include <filesystem>

#include "metric_logger.hh"
#include "loss_info.hh"
#include "frame.hh"
#include "loss_computer.hh"
#include "loss_metric_computer.hh"
#include "frame_log.hh"

namespace fs = std::filesystem;

class FrameLogger
{
public:
  FrameLogger(std::string save_folder) {
    save_folder_ = fs::current_path().append(save_folder);
  };

  void add_frame(const Frame & frame, const FrameLog& frame_log);
  std::deque<FrameLog> get_frame_logs();

  ~FrameLogger() {
    fs::create_directories(save_folder_);
    auto temp_folder_1 = save_folder_;
    auto save_name_1 = temp_folder_1.append("received_frames_loss_info_before_fec");
    std::fstream file1;
    file1.open(save_name_1.string(), std::ios::out);
    if (!file1) {
      std::cerr << "File " + save_name_1.string()+ " could not be opened\n";
      return;
    }
    LossComputer lc_before_fec(frames_);
    auto li_before_fec = lc_before_fec.get_loss_info();
    file1 << li_before_fec;
    file1.close();

    LossMetricComputer lmc(li_before_fec, 0);
    auto burstIdxs = lmc.getBurstIndices(li_before_fec.frame_losses);

    auto tf = save_folder_;
    auto sn = tf.append("burst_idx");
    std::fstream f;
    f.open(sn.string(), std::ios::out);
    if (!f) {
      std::cerr << "File " + sn.string()+ " could not be opened\n";
      return;
    }
    size_t idx = 0;
    f << "{\"burst_start\":[";
    for (auto const& b : burstIdxs) {
      f << b.first;
      if (idx != burstIdxs.size()-1) {
        f << ",";
      }
      idx++;
    }
    f << "],\"burst_end\":[";
    idx = 0;
    for (auto const& b : burstIdxs) {
      f << b.second;
      if (idx != burstIdxs.size()-1) {
        f << ",";
      }
      idx++;
    }
    f << "]}";
    f.close();

    auto temp_folder_1_1 = save_folder_;
    auto save_name_1_1 = temp_folder_1_1.append("received_frames_loss_info_after_fec");
    std::fstream file1_1;
    file1_1.open(save_name_1_1.string(), std::ios::out);
    if (!file1_1) {
      std::cerr << "File " + save_name_1_1.string()+ " could not be opened\n";
      return;
    }

    LossComputer lc_after_fec(frames_, false);
    auto li_after_fec = lc_after_fec.get_loss_info();
    file1_1 << li_after_fec;

    file1_1.close();


    uint32_t total_frames = frames_.size();
    uint32_t num_decoded = 0;
    for (auto const & it : frames_)
    {
      if (it.is_decoded())
      {
        num_decoded += 1;
      }
    }

    auto temp_folder_2 = save_folder_;
    auto save_name_2 = temp_folder_2.append("received_frames_log_summary");
    std::fstream file2;
    file2.open(save_name_2.string(), std::ios::out);
    if (!file2) {
      std::cerr << "File " + save_name_2.string()+ " could not be opened\n";
      return;
    }

    file2 << "total_frames:" << total_frames << "\n";
    file2 << "decoded_frames:" << num_decoded << "\n";
    file2.close();

    auto temp_folder_3 = save_folder_;
    auto save_name_3 = temp_folder_3.append("received_frames_log");
    std::fstream file3;
    file3.open(save_name_3.string(), std::ios::out);
    if (!file3) {
      std::cerr << "File " + save_name_3.string()+ " could not be opened\n";
      return;
    }

    file3 << FrameLog::header;

    for (auto const & it : frames_log_)
    {
      file3 << it;
    }

    file3.close();
  }

private:
  fs::path save_folder_;
  std::deque<Frame> frames_;
  std::deque<FrameLog> frames_log_;
};

#endif /* FRAME_LOGGER_HH */
