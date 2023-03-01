#ifndef LOGGER_HH
#define LOGGER_HH

#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

#include "metric_logger.hh"
#include "frame.hh"
#include "logging/frame_log.hh"
#include "logging/frame_logger.hh"

#define LOGV(variable) std::string(#variable) + ":" + std::to_string(variable)

namespace fs = std::filesystem;
using namespace std::chrono;

class Logger
{
public:
  Logger(std::string const& save_folder, std::vector<MetricLogger *> metricLoggers, FrameLogger* frame_logger = nullptr) :
      metricLoggers_(metricLoggers), frame_logger_(frame_logger) {
        save_folder_ = fs::current_path().append(save_folder);
      };

  void begin_function(const std::string & fname);

  std::string get_extra_variables() {
    return "";
  }

  template<typename First, typename... Args>
  std::string get_extra_variables(First first, Args...args)
  {
    return std::to_string(first) + "," + get_extra_variables(args...);
  }

  template<typename... Args>
  std::string get_extra_variables(std::string first, Args...args)
  {
    return first  + "," + get_extra_variables(args...);
  }

  void end_function()
  {
    call_end_function("");
  }

  template<typename... Args>
  void end_function(Args...args)
  {
    auto extra_var = get_extra_variables(args...);
    call_end_function(extra_var);
  }

  void add_frame(const Frame & frame, const FrameLog & frame_log);

  std::deque<FrameLog> get_frame_logs();

  template<typename...Args>
  void log_event(const std::string& event_name, Args...args)
  {
    auto extra_info = get_extra_variables(args...);
    events_[event_name].emplace_back(extra_info, duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now().time_since_epoch()).count());
  }

  template<typename...Args>
  void log_value(const std::string& value_name, Args...args)
  {
    auto extra_info = get_extra_variables(args...);
    values_[value_name].push_back(extra_info);
  }

  ~Logger();

private:
  void call_end_function(std::string extra_var)
  {
    for (uint16_t ind = 0; ind < metricLoggers_.size(); ind++) {
      (*metricLoggers_.at(ind)).end_function(extra_var);
    }
  }

private:
  fs::path save_folder_;
  std::vector<MetricLogger *> metricLoggers_;
  FrameLogger* frame_logger_;
  std::unordered_map<std::string, std::vector<std::pair<std::string, uint64_t>>> events_;
  std::unordered_map<std::string, std::vector<std::string>> values_;
};

#endif /* LOGGER_HH */
