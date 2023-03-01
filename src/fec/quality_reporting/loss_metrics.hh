#ifndef LOSS_METRICS_HH
#define LOSS_METRICS_HH

#include <stdint.h>
#include <ostream>

struct LossMetrics
{
  float burst_density;
  float frame_burst_density;
  float gap_density;
  float frame_gap_density;
  float mean_burst_length;
  float mean_frame_burst_length;
  float loss_fraction;
  float frame_loss_fraction;
  float multi_frame_loss_fraction;
  float mean_guardspace_length;
  float mean_frame_guardspace_length;
  float multi_frame_insufficient_guardspace;
  float qr;

  friend std::ostream& operator<<(std::ostream& oss, const LossMetrics& lm)
  {
    oss << "burst_density:" << lm.burst_density
        << " frame_burst_density:" << lm.frame_burst_density
        << " gap_density:" << lm.gap_density
        << " frame_gap_density:" << lm.frame_gap_density
        << " mean_burst_length:" << lm.mean_burst_length
        << " mean_frame_burst_length:" << lm.mean_frame_burst_length
        << " loss_fraction:" << lm.loss_fraction
        << " frame_loss_fraction:" << lm.frame_loss_fraction
        << " multi_frame_loss_fraction:" << lm.multi_frame_loss_fraction
        << " mean_guardspace_length:" << lm.mean_guardspace_length
        << " mean_frame_guardspace_length:" << lm.mean_frame_guardspace_length
        << " multi_frame_insufficient_guardspace:" << lm.multi_frame_insufficient_guardspace
        << " qr:" << lm.qr << "\n";

    return oss;
  }
};

inline bool operator==(const LossMetrics & lhs, const LossMetrics & rhs)
{
  if (lhs.burst_density != rhs.burst_density) { return false; };
  if (lhs.frame_burst_density != rhs.frame_burst_density) { return false; };
  if (lhs.gap_density != rhs.gap_density) { return false; };
  if (lhs.frame_gap_density != rhs.frame_gap_density) { return false; };
  if (lhs.mean_burst_length != rhs.mean_burst_length) { return false; };
  if (lhs.mean_frame_burst_length != rhs.mean_frame_burst_length) {
      return false; };
  if (lhs.loss_fraction != rhs.loss_fraction) { return false; };
  if (lhs.frame_loss_fraction != rhs.frame_loss_fraction) { return false; };
  if (lhs.multi_frame_loss_fraction != rhs.multi_frame_loss_fraction) {
      return false; };
  if (lhs.mean_guardspace_length != rhs.mean_guardspace_length) { return false; };
  if (lhs.mean_frame_guardspace_length != rhs.mean_frame_guardspace_length) {
      return false; };
  if (lhs.multi_frame_insufficient_guardspace != rhs.multi_frame_insufficient_guardspace) {
      return false; };
  if (lhs.qr != rhs.qr) { return false; };
  return true;
}

#endif /* LOSS_METRICS_HH */
