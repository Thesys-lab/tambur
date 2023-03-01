#ifndef LOSS_METRIC_COMPUTER_HH
#define LOSS_METRIC_COMPUTER_HH

#include <vector>
#include <map>

#include "frame.hh"
#include "loss_metrics.hh"
#include "loss_info.hh"

class LossMetricComputer
{
public:
  LossMetricComputer(const LossInfo & lossInfo, uint8_t qr,
      uint8_t gMin_packet = 1, uint8_t gMin_frame = 3,
      uint8_t multi_frame_burst = 2, uint8_t multi_frame_minimal_guardspace = 3
      ) : lossInfo_(lossInfo), qr_(float(qr)), gMin_packet_(gMin_packet),
      gMin_frame_(gMin_frame), multi_frame_burst_(multi_frame_burst),
      multi_frame_minimal_guardspace_(multi_frame_minimal_guardspace) {};

  LossMetrics compute_loss_metrics();

  float compute_mean_length(const std::map<uint16_t, uint16_t> & stats);

  float compute_loss_fraction(const std::vector<bool> & losses);

  float compute_multi_frame_insufficient_guardspace(
      const std::map<uint16_t, uint16_t> & guardspaces,
      uint16_t multi_frame_minimal_guardspace);

  float compute_multi_frame_sufficient_guardspace(
      const std::map<uint16_t, uint16_t> & guardspaces,
      uint16_t multi_frame_minimal_guardspace);

  std::pair<std::map<uint16_t, uint16_t>, std::map<uint16_t, uint16_t>>
      get_bursts_and_guardspaces(const std::vector<bool> & losses);

  std::pair<float, float> getRFCDensities(const std::vector<bool> & losses,
      uint16_t gMin);

  float getDensity(const std::vector<std::vector<bool>> vals);

  std::vector<std::pair<uint16_t, uint16_t>> eliminate_singleton_bursts(
      const std::vector<std::pair<uint16_t, uint16_t>> & burstIndices);

  std::vector<std::pair<uint16_t, uint16_t>> getBurstIndices(
      const std::vector<bool> & losses, uint16_t gMin = 1);

  std::pair<uint16_t, uint16_t> consecutive_burst_info(
      const std::vector<bool> & losses,
      const std::vector<uint16_t> & frame_indices );

  std::pair<std::vector<uint16_t>, std::vector<uint16_t>>
      get_positions_consecutive_losses(
      const std::vector<bool> & losses,
      const std::vector<uint16_t> & frame_indices,
      uint16_t min_num_consecutive_frames_to_evaluate);

  float compute_multi_frame_loss_fraction(const std::vector<bool> & losses,
      const std::vector<uint16_t> & frame_indices,
      uint16_t min_num_consecutive_frames_to_evaluate);

  std::tuple<float, float, float> get_window_metrics(
      const LossInfo & lossInfo);

private:
  LossInfo lossInfo_;
  float qr_;
  uint8_t gMin_packet_;
  uint8_t gMin_frame_;
  uint8_t multi_frame_burst_;
  uint8_t multi_frame_minimal_guardspace_;
  LossMetrics lossMetrics_;
};

#endif /* LOSS_METRIC_COMPUTER_HH */
