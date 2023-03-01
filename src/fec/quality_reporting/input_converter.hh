#ifndef INPUT_CONVERTER_HH
#define INPUT_CONVERTER_HH

#include <string>
#include <vector>
#include <deque>
#include <optional>

#include "loss_metrics.hh"

static constexpr uint8_t NUM_METRICS = 13;

class InputConverter
{
public:
  InputConverter(std::string means_fname,
      std::string vars_fname, uint16_t num_indicators = 3,
      float epsilon = 0.00001);

  void update_all_loss_metrics(const LossMetrics & metrics);

  std::optional<std::vector<double>> get_inputs();

private:
  uint16_t num_indicators_;
  std::vector<float> means_;
  std::vector<float> vars_;
  float epsilon_;
  std::deque<std::vector<double>> allLossMetrics_;

  void load(const std::string & fname, std::vector<float> & vals);

  std::vector<double> normalize(const std::vector<double> & allMetrics);
};

#endif /* INPUT_CONVERTER_HH */
