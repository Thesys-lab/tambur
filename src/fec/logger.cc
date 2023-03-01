#include "logger.hh"

using namespace std;


void Logger::begin_function(const std::string & fname)
{
  for (uint16_t ind = 0; ind < metricLoggers_.size(); ind++) {
    (*metricLoggers_.at(ind)).begin_function(fname);
  }
}

void Logger::add_frame(const Frame & frame, const FrameLog& frame_log)
{
  if (frame_logger_) {
    frame_logger_->add_frame(frame, frame_log);
  }
}

std::deque<FrameLog> Logger::get_frame_logs()
{
  if (frame_logger_) {
    return  frame_logger_->get_frame_logs();
  }
  return {};
}

Logger::~Logger()
{
  fs::create_directories(save_folder_);
  for (auto const& evt : events_) {
    auto temp_folder = save_folder_;
    auto save_name = temp_folder.append(evt.first);
    std::cerr << "File " + save_name.string() << endl;
    std::fstream file;
    file.open(save_name.string(), std::ios::out);
    if (!file) {
      std::cerr << "File " + save_name.string()+ " could not be opened\n";
      return;
    }
    for (auto t : evt.second)
    {
      file << std::to_string(t.second) << "," << t.first << "\n";
    }
    file.close();
  }

  for (auto const& val : values_) {
    auto temp_folder = save_folder_;
    auto save_name = temp_folder.append(val.first);
    std::cerr << "File " + save_name.string() << endl;
    std::fstream file;
    file.open(save_name.string(), std::ios::out);
    if (!file) {
      std::cerr << "File " + save_name.string()+ " could not be opened\n";
      return;
    }
    for (auto t : val.second)
    {
      file << t << "\n";
    }
    file.close();
  }
}
