#ifndef TIMING_LOGGER_HH
#define TIMING_LOGGER_HH

#include <string>
#include <stack>
#include <deque>
#include <chrono>
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <iostream>
#include <fstream>

#include "metric_logger.hh"
#include "timestamp.hh"

namespace fs = std::filesystem;

class TimingLogger : public MetricLogger
{
public:
  TimingLogger(std::string save_folder) {
    save_folder_ = fs::current_path().append(save_folder);
  };

  void begin_function(const std::string & fname) override;

  void end_function(std::string extra_var) override;

  ~TimingLogger() {
    fs::create_directories(save_folder_);

    for (auto it : timings_) {
      auto temp_folder = save_folder_;
      auto save_name = temp_folder.append(it.first);
      std::fstream file;
      file.open(save_name.string(), std::ios::out);
      if (!file) {
        std::cerr << "File " + save_name .string()+ " could not be opened\n";
        return;
      }
      for (auto t : it.second)
      {
        file << std::to_string(t.second) << "," << t.first << "\n";
      }
      file.close();
    }
  }

private:
  fs::path save_folder_;
  std::stack<uint64_t> start_timings_;
  std::stack<std::string> fnames_;
  std::unordered_map<std::string, std::vector<std::pair<std::string, uint64_t>>> timings_;
};

#endif /* TIMING_LOGGER_HH */
