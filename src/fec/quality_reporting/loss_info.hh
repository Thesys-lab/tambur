#ifndef LOSS_INFO_HH
#define LOSS_INFO_HH

#include <vector>
#include <ostream>

struct LossInfo
{
  std::vector<uint16_t> indices;
  std::vector<bool> packet_losses;
  std::vector<bool> frame_losses;

  friend std::ostream& operator<<(std::ostream& oss, const LossInfo& li)
  {
    oss << "{\n";
    oss << "\"indices\": [";
    size_t idx = 0;
    for (auto a : li.indices) {
      oss << a;
      if (idx != li.indices.size()-1) {
        oss << ",";
      }
      idx++;
    }
    oss << "],\n";

    oss << "\"packet_losses\": [";
    idx = 0;
    for (auto a : li.packet_losses) {
      oss << a;
      if (idx != li.packet_losses.size()-1) {
        oss << ",";
      }
      idx++;
    }
    oss << "],\n";

    oss << "\"frame_losses\": [";
    idx = 0;
    for (auto a : li.frame_losses) {
      oss << a;
      if (idx != li.frame_losses.size()-1) {
        oss << ",";
      }
      idx++;
    }
    oss << "]\n";
    oss << "}";
    return oss;
  }
};



#endif /* LOSS_INFO_HH */
