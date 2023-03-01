#include "helpersQR.hh"
#include "loss_metric_computer.hh"

using namespace std;

bool test_input_converter_test_threshold_num_indicators(string directory)
{
  float epsilon = 0.00001;
  string means = directory;
  for (auto el : "/Tambur_test_means.txt") { means.push_back(el); }
  string vars = directory;
  for (auto el: "/Tambur_test_vars.txt") { vars.push_back(el); }
  InputConverter inputConverter(means, vars, 3, epsilon);
  for (uint16_t j = 0; j < 2; j++) {
    inputConverter.update_all_loss_metrics(LossMetrics());
  }
  if (inputConverter.get_inputs().has_value()) { return false; }
  inputConverter.update_all_loss_metrics(LossMetrics());
  if (!inputConverter.get_inputs().has_value()) { return false; }
  return true;
}

bool test_input_converter_test_input_conversions(string directory)
{
  float epsilon = 0.00001;
  vector<double> vals;
  vector<LossMetrics> all_metrics;
  InputConverter inputConverter(directory + "/Tambur_test_means.txt",
      directory + "/Tambur_test_vars.txt", 3, epsilon);
  for (uint16_t j = 0; j < 3; j++) {
    LossMetrics metrics = LossMetricComputer(
        lossInfosForTests.at(j), 1, 3, 3, 2, 2).compute_loss_metrics();
    inputConverter.update_all_loss_metrics(metrics);
    all_metrics.push_back(metrics);
  }
  uint16_t pos = 0;
  for (auto metric : all_metrics) {
    vals.push_back((
        metric.burst_density- test_means.at(pos)) / sqrt(test_vars.at(pos) +
        epsilon));
    pos += 1;
  }
  for (auto metric : all_metrics) {
    vals.push_back((
        metric.frame_burst_density - test_means.at(pos)) /
        sqrt(test_vars.at(pos) + epsilon));
    pos += 1;
  }
  for (auto metric : all_metrics) {
    vals.push_back((
        metric.gap_density - test_means.at(pos)) / sqrt(test_vars.at(pos) +
        epsilon));
    pos += 1;
  }
  for (auto metric : all_metrics) {
    vals.push_back((
        metric.frame_gap_density - test_means.at(pos)) / sqrt(test_vars.at(pos)
        + epsilon));
    pos += 1;
  }
   for (auto metric : all_metrics) {
    vals.push_back((
        metric.mean_burst_length - test_means.at(pos)) / sqrt(test_vars.at(pos)
        + epsilon));
    pos += 1;
  }
   for (auto metric : all_metrics) {
    vals.push_back((
        metric.mean_frame_burst_length - test_means.at(pos)) /
        sqrt(test_vars.at(pos) + epsilon));
    pos += 1;
  }
  for (auto metric : all_metrics) {
    vals.push_back((
        metric.loss_fraction - test_means.at(pos)) / sqrt(test_vars.at(pos) +
        epsilon));
    pos += 1;
  }
  for (auto metric : all_metrics) {
    vals.push_back((
        metric.frame_loss_fraction - test_means.at(pos)) /
        sqrt(test_vars.at(pos) + epsilon));
    pos += 1;
  }
  for (auto metric : all_metrics) {
    vals.push_back((
        metric.multi_frame_loss_fraction - test_means.at(pos)) /
        sqrt(test_vars.at(pos) + epsilon));
    pos += 1;
  }
  for (auto metric : all_metrics) {
    vals.push_back((
        metric.mean_guardspace_length - test_means.at(pos)) /
        sqrt(test_vars.at(pos) + epsilon));
    pos += 1;
  }
  for (auto metric : all_metrics) {
    vals.push_back((
        metric.mean_frame_guardspace_length - test_means.at(pos)) /
        sqrt(test_vars.at(pos) + epsilon));
    pos += 1;
  }
  for (auto metric : all_metrics) {
    vals.push_back((
        metric.multi_frame_insufficient_guardspace - test_means.at(pos)) /
        sqrt(test_vars.at(pos) + epsilon));
      pos += 1;
  }
  for (auto metric : all_metrics) {
    vals.push_back((
        float(metric.qr) - test_means.at(pos))/ sqrt(test_vars.at(pos) +
        epsilon));
    pos += 1;
  }
  auto test_vals = inputConverter.get_inputs().value();
  if (vals.size() != test_vals.size()) {
    return false;
  }
  for (uint16_t i = 0; i < vals.size(); i++) {
    if (abs(vals.at(i) - test_vals.at(i)) > 0.00001) { return false; }
  }
  return true;
}

int main(int argc, char * argv[])
{
  string directory;
  if (argc != 2) {
    directory = string("src/tests");
  }
  else {
    directory = string(argv[1]);
  }
  assert(test_input_converter_test_threshold_num_indicators(directory));
  printf("test_input_converter_test_threshold_num_indicators: passed\n");
  assert(test_input_converter_test_input_conversions(directory));
  printf("test_input_converter_test_input_conversions: passed\n");
}
