#ifndef FRAME_LOG_HH
#define FRAME_LOG_HH

#include <string>
#include <stack>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>


// Struct to hold all logging info for a corresponding frame
struct FrameLog
{
  FrameLog(uint16_t frame_num) : frame_num_(frame_num) {};

  uint16_t frame_num_;
  bool not_decoded_{true};
  uint16_t decoded_after_frames_{0};
  uint64_t time_spent_decode_{0};
  uint64_t first_ts_{0};
  uint64_t first_data_ts_{0};
  uint64_t first_par_ts_{0};
  uint64_t last_data_ts_{0};
  uint64_t last_par_ts_{0};
  uint64_t decode_ts_{0};
  bool all_data_received_{false};

  static inline std::string header = "frame_num_,not_decoded_,decoded_after_frames_,time_spent_decode_,first_ts_,first_data_ts_,first_par_ts_,last_data_ts_,last_par_ts_,decode_ts_,all_data_received\n";



  friend std::ostream& operator<<(std::ostream& oss, const FrameLog& fl)
  {
    oss << fl.frame_num_ << ","
        << fl.not_decoded_ << ","
        << fl.decoded_after_frames_ << ","
        << fl.time_spent_decode_ << ","
        << fl.first_ts_ << ","
        << fl.first_data_ts_ << ","
        << fl.first_par_ts_ << ","
        << fl.last_data_ts_ << ","
        << fl.last_par_ts_ << ","
        << fl.decode_ts_ << ","
        << fl.all_data_received_
        << "\n";
    return oss;
  }
};

#endif /* FRAME_LOGGER_HH */
