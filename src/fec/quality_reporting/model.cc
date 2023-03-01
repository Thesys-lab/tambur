#include <iostream>
#include <stdexcept>
#include <memory>
#include <cmath>

#include "model.hh"

using namespace std;

Model::Model(string model_fname, uint8_t num_qrs_no_reduce) :
    num_qrs_no_reduce_(num_qrs_no_reduce)
{
  try {
    module_ = torch::jit::load(model_fname);
  }
  catch (...) {
    throw runtime_error("error loading the model");
  }
}

at::Tensor Model::get_prediction(const vector<double> inputs)
{
  vector<torch::jit::IValue> torch_inputs;
  torch_inputs.push_back(torch::tensor(inputs));
  auto output = module_.forward(torch_inputs);
  at::Tensor prediction = output.toTensor();
  return prediction;
}

uint8_t Model::get_quality_report(InputConverter & inputConverter,
    const LossMetrics & metrics)
{
  auto inputs = inputConverter.get_inputs();
  if (!inputs.has_value()) { return uint8_t(round(metrics.qr)) % num_qrs_no_reduce_; }
  at::Tensor prediction = get_prediction(inputs.value());
  uint8_t qr = convert_prediction_to_qr(prediction, uint8_t(round(metrics.qr)));
  return qr;
}

uint8_t Model::convert_prediction_to_qr(at::Tensor prediction,
    uint8_t original_qr)
{
  if (original_qr == 0) { return original_qr; }
  bool low_BW = prediction[1].item<double>() >= prediction[0].item<double>();
  return (original_qr % num_qrs_no_reduce_) + (num_qrs_no_reduce_ -1) * (low_BW
      ? 1 : 0);
}
