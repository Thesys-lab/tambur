#ifndef MODEL_HH
#define MODEL_HH

#include <torch/script.h>
#include <string>
#include <deque>
#include <iostream>
#include <memory>

#include "loss_metrics.hh"
#include "input_converter.hh"

class Model
{
public:
  Model(std::string model_fname, uint8_t num_qrs_no_reduce);

  at::Tensor get_prediction(const std::vector<double> inputs);

  uint8_t get_quality_report(InputConverter & inputConverter,
      const LossMetrics & metrics);

private:
  uint8_t num_qrs_no_reduce_;
  torch::jit::script::Module module_;

  uint8_t convert_prediction_to_qr(at::Tensor prediction, uint8_t original_qr);
};

#endif /* MODEL_HH */
