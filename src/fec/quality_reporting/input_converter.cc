#include <iostream>
#include <fstream>
#include <filesystem>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <cmath>

#include "input_converter.hh"

using namespace std;

InputConverter::InputConverter(string means_fname,
    string vars_fname, uint16_t num_indicators, float epsilon) :
    num_indicators_(num_indicators), epsilon_(epsilon)
{
  load(means_fname, means_);
  load(vars_fname, vars_);
}

void InputConverter::load(const std::string &fname, std::vector<float> &vals)
{
  ifstream file(fname);
  if (!file.is_open())
  {
    throw runtime_error("File " + fname + " could not be opened\n");
  }
  vals.reserve(num_indicators_ * NUM_METRICS);
  string file_val;
  file >> file_val;
  uint64_t pos = 0;
  uint64_t last_pos = 0;
  while (file_val.find('!', pos) != string::npos)
  {
    pos = file_val.find('!', pos);
    if (pos >= file_val.size())
    {
      break;
    }
    vals.push_back(stof(string(file_val.begin() + last_pos, file_val.begin() + pos)));
    last_pos = pos + 1;
    pos = pos + 1;
  }
  file.close();
  if (vals.size() != num_indicators_ * NUM_METRICS)
  {
    throw runtime_error("File " + fname + " has an invalid number of values\n");
  }
}

void InputConverter::update_all_loss_metrics(const LossMetrics & metrics)
{
  allLossMetrics_.emplace_back( vector<double>{ metrics.burst_density,
      metrics.frame_burst_density, metrics.gap_density,
      metrics.frame_gap_density, metrics.mean_burst_length,
      metrics.mean_frame_burst_length, metrics.loss_fraction,
      metrics.frame_loss_fraction, metrics.multi_frame_loss_fraction,
      metrics.mean_guardspace_length, metrics.mean_frame_guardspace_length,
      metrics.multi_frame_insufficient_guardspace, metrics.qr });
  if (allLossMetrics_.size() > num_indicators_)
  {
    allLossMetrics_.pop_front();
  }
}

vector<double> InputConverter::normalize(const vector<double> & allMetrics)
{
  if (allMetrics.size() != num_indicators_ * NUM_METRICS) {
    throw runtime_error("Incorrect number of inputs to model\n");
  }
  vector<double> normalized_vals;
  normalized_vals.reserve(num_indicators_*NUM_METRICS);
  for (uint16_t pos = 0; pos < num_indicators_*NUM_METRICS; pos++) {
    normalized_vals.push_back((allMetrics.at(pos) -  means_.at(pos))
        / sqrt(vars_.at(pos) + epsilon_));
  }
  return normalized_vals;
}

optional<vector<double>> InputConverter::get_inputs()
{
  optional<vector<double>> vals;
  if (allLossMetrics_.size() < num_indicators_)
  {
    return vals;
  }
  vals = vector<double>();
  vals->reserve(num_indicators_ * NUM_METRICS);
  for (uint16_t position = 0; position < NUM_METRICS; position++)
  {
    for (uint16_t index = 0; index < num_indicators_; index++)
    {
      vals->push_back(allLossMetrics_.at(index).at(position));
    }
  }
  auto normalized_vals = normalize(vals.value());
  return normalized_vals;
}
