#include <cassert>
#include <cmath>
#include <numeric>
#include "loss_metric_computer.hh"

using namespace std;

LossMetrics LossMetricComputer::compute_loss_metrics()
{
  auto packetRFC = getRFCDensities(lossInfo_.packet_losses,gMin_packet_);
  auto frameRFC = getRFCDensities(lossInfo_.frame_losses,gMin_frame_);
  float burst_density = packetRFC.first;
  float frame_burst_density = frameRFC.first;
  float gap_density = packetRFC.second;
  float frame_gap_density = frameRFC.second;
  auto bursts_and_guardspaces_vals = get_bursts_and_guardspaces(lossInfo_.packet_losses);
  float mean_burst_length = compute_mean_length(
        bursts_and_guardspaces_vals.first);
  float mean_guardspace_length = compute_mean_length(
        bursts_and_guardspaces_vals.second);

  auto bursts_and_guardspaces_frame_vals = get_bursts_and_guardspaces(
        lossInfo_.frame_losses);
  float mean_frame_burst_length = compute_mean_length(
      bursts_and_guardspaces_frame_vals.first);
  float mean_frame_guardspace_length = compute_mean_length(
      bursts_and_guardspaces_frame_vals.second);
  float multi_frame_insufficient_guardspace =
      compute_multi_frame_insufficient_guardspace(
      bursts_and_guardspaces_frame_vals.second,
      multi_frame_minimal_guardspace_);

  float loss_fraction = compute_loss_fraction(lossInfo_.packet_losses);
  float frame_loss_fraction = compute_loss_fraction(lossInfo_.frame_losses);
  float multi_frame_loss_fraction = compute_multi_frame_loss_fraction(
        lossInfo_.packet_losses, lossInfo_.indices, multi_frame_burst_);

  return LossMetrics { burst_density, frame_burst_density, gap_density,
      frame_gap_density, mean_burst_length, mean_frame_burst_length,
      loss_fraction, frame_loss_fraction, multi_frame_loss_fraction,
      mean_guardspace_length, mean_frame_guardspace_length,
      multi_frame_insufficient_guardspace, qr_ };
}

float LossMetricComputer::compute_mean_length(
    const map<uint16_t, uint16_t> & stats)
{
  float total = 0;
  float count_occurrences = 0;
  for (pair<uint16_t, uint16_t> element : stats) {
    total += element.first*element.second;
    count_occurrences += element.second;
  }
  return count_occurrences > 0? total / count_occurrences : 0.0;
}

float LossMetricComputer::compute_loss_fraction(
    const vector<bool> &losses)
{
  if (losses.size() == 0) { return 0.0; }
  float num_lost = 0;
  for (auto is_lost : losses) { num_lost += float(is_lost); }
  return num_lost / float(losses.size());
}

float LossMetricComputer::compute_multi_frame_insufficient_guardspace(
    const map<uint16_t, uint16_t> & guardspaces,
    uint16_t multi_frame_minimal_guardspace)
{
  if (guardspaces.empty()) { return 0.0; }
  float num_smaller = 0.0;
  float total = 0.0;
  for (auto element : guardspaces) {
    if (element.first < multi_frame_minimal_guardspace) {
      num_smaller += element.second;
    }
    total += element.second;
  }
  return num_smaller / total;
}

float LossMetricComputer::compute_multi_frame_sufficient_guardspace(
    const map<uint16_t, uint16_t> & guardspaces,
    uint16_t multi_frame_minimal_guardspace)
{
  return 1.0 - compute_multi_frame_insufficient_guardspace(guardspaces,
      multi_frame_minimal_guardspace);
}

pair<map<uint16_t, uint16_t>, map<uint16_t, uint16_t>>
    LossMetricComputer::get_bursts_and_guardspaces(const vector<bool> & losses)
{
  map<uint16_t, uint16_t> bursts;
  map<uint16_t, uint16_t> guardspaces;
  uint16_t start = 0;
  bool currBurst = false;
  uint16_t bufferStart = 0;
  uint16_t length = 0;
  for (uint16_t i = 0; i < losses.size(); i++) {
      if (currBurst) {
        if (losses.at(i) == 0) {
          length = i - start;
          if (bursts.find(length) == bursts.end()) { bursts[length] = 0; }
          bursts[length]++;
          bufferStart = i;
        }
      }
      else {
        if (losses.at(i)) {
          start = i;
          length = i - bufferStart;
          if (length) {
            if (guardspaces.find(length) == guardspaces.end()) {
              guardspaces[length] = 0;
            }
            guardspaces[length]++;
          }
        }
      }
      currBurst = (losses.at(i) == 1);
  }
  if (losses.size() > 0 and currBurst) {
    length = losses.size() - start;
    if (bursts.find(length) == bursts.end()) { bursts[length] = 0; }
    bursts[length]++;
  }
  else if (losses.size() > 0) {
    length = losses.size() - bufferStart;
    if (guardspaces.find(length) == guardspaces.end()) {
      guardspaces[length] = 0;
    }
    guardspaces[length]++;
  }
  return { bursts, guardspaces };
}

pair<float, float> LossMetricComputer::getRFCDensities(
      const vector<bool> & losses, uint16_t gMin)
{
  assert(gMin > 0);
  float numLost = 0.0;
  for (auto element : losses) { numLost += float(element); }
  if (numLost == 0.0) { return { 0.0, 0.0 }; }
  auto burstIndicesWithLengthOne = getBurstIndices(losses, gMin);
  auto burstIndices = eliminate_singleton_bursts(burstIndicesWithLengthOne);
  vector<vector<bool>> bursts;
  bursts.reserve(burstIndices.size());
  for (auto element : burstIndices) {
    vector<bool> burst(losses.begin() + element.first,
        losses.begin() + 1 + element.second);
    bursts.push_back(burst);
  }
  if (bursts.empty()) { return { 0.0, numLost / max(float(1.0), float(losses.size())) }; }
  vector<pair<uint16_t, uint16_t>> guardspaceIndices;
  vector<vector<bool>> guardspaces;
  if (burstIndices.front().first > 0) {
    vector<bool> guardspace(losses.begin(),
        losses.begin() + burstIndices.front().first);
    guardspaces.push_back(guardspace);
  }
  auto end_burst = burstIndices.front().second;
  for (uint16_t j = 1; j < burstIndices.size(); j++) {
    vector<bool> guardspace(losses.begin() + end_burst + 1,
        losses.begin() + burstIndices.at(j).first);
    guardspaces.push_back(guardspace);
    guardspaceIndices.push_back({ end_burst + 1,
        burstIndices.at(j).first - 1 });
    end_burst = burstIndices.at(j).second;
  }
  if (uint16_t(end_burst + 1) < losses.size()) {
    vector<bool> guardspace(losses.begin() + end_burst + 1, losses.end());
    guardspaces.push_back(guardspace);
    guardspaceIndices.push_back({ end_burst + 1, losses.size() - 1 });
  }
  return { getDensity(bursts), getDensity(guardspaces) };
}

float LossMetricComputer::getDensity(const vector<vector<bool>> vals)
{
  float density = 0;
  float total_length = 0.0;
  for (auto period : vals) {
    for (auto loss : period) { density += float(loss); }
    total_length += float(period.size());
  }
  return density / max(float(1.0), total_length);
}

vector<pair<uint16_t, uint16_t>> LossMetricComputer::eliminate_singleton_bursts(
    const vector<pair<uint16_t, uint16_t>> & burstIndices)
{
  vector<pair<uint16_t, uint16_t>> includedBurstIndices;
  for (auto element : burstIndices) {
    if (element.second > element.first) {
      includedBurstIndices.push_back(element);
    }
  }
  return includedBurstIndices;
}

vector<pair<uint16_t, uint16_t>> LossMetricComputer::getBurstIndices(
    const vector<bool> & losses, uint16_t gMin)
{
  vector<pair<uint16_t, uint16_t>> burstIndices;
  uint16_t length = losses.size();
  uint16_t i = 0;
  while (i < length) {
    if (!losses.at(i)) { i += 1; }
    else {
      auto j = i;
      while (j < length and accumulate(losses.begin() + j,
          losses.begin() + min(uint16_t(j + gMin), length), 0)) {
        j++;
      }
      burstIndices.push_back({ i, j - 1});
      i = j;
    }
  }
  return burstIndices;
}

pair<uint16_t, uint16_t> LossMetricComputer::consecutive_burst_info(
    const vector<bool> & losses, const vector<uint16_t> & frame_indices)
{
  uint16_t num_frames_in_burst = 0;
  uint16_t num_pkts_for_frames_in_burst = 0;
  uint16_t length = frame_indices.size();
  uint16_t index = 0;
  bool curr_burst = true;
  while (index < length and curr_burst) {
    auto start_losses = frame_indices.at(index);
    index++;
    auto end_frame = index < length? frame_indices.at(index) :
        losses.size();
    curr_burst = curr_burst and (accumulate(losses.begin() + start_losses,
        losses.begin() + end_frame, 0) > 0);
    if (curr_burst) {
      num_frames_in_burst++;
      num_pkts_for_frames_in_burst += end_frame - start_losses;
    }
  }
  return { num_frames_in_burst, num_pkts_for_frames_in_burst };
}

pair<vector<uint16_t>, vector<uint16_t>>
    LossMetricComputer::get_positions_consecutive_losses(
    const vector<bool> & losses, const vector<uint16_t> & frame_indices,
    uint16_t min_num_consecutive_frames_to_evaluate)
{
  assert(min_num_consecutive_frames_to_evaluate > 0);
  vector<uint16_t> positions;
  vector<uint16_t> lengths;
  uint16_t index_pos = 0;
  uint16_t num_indices = frame_indices.size();
  while (index_pos <= num_indices - min_num_consecutive_frames_to_evaluate) {
    auto index = frame_indices.at(index_pos);
    vector<bool> packet_losses(losses.begin() + index, losses.end());
    vector<uint16_t> adjusted_indices;
    adjusted_indices.reserve(frame_indices.size());
    for (uint16_t index2 = index_pos; index2 < num_indices; index2++) {
      adjusted_indices.push_back(frame_indices.at(index2) - index);
    }
    auto info = consecutive_burst_info(packet_losses,
        adjusted_indices);
    if (info.first >= min_num_consecutive_frames_to_evaluate) {
      positions.push_back(index);
      lengths.push_back(info.second);
      index_pos += info.first;
    }
    else { index_pos++; }
  }
  return { positions, lengths };
}

float LossMetricComputer::compute_multi_frame_loss_fraction(
    const vector<bool> & losses, const vector<uint16_t> & frame_indices,
    uint16_t min_num_consecutive_frames_to_evaluate)
{
  assert(min_num_consecutive_frames_to_evaluate > 0);
  auto positions_losses = get_positions_consecutive_losses(
      losses, frame_indices, min_num_consecutive_frames_to_evaluate);
  uint16_t num_occurrences_of_consecutive_losses =
      positions_losses.first.size();
  float average_loss_frac = 0.0;
  for (uint16_t i = 0; i < num_occurrences_of_consecutive_losses; i++) {
    auto position = positions_losses.first.at(i);
    auto num_pkts = positions_losses.second.at(i);
    average_loss_frac += float(accumulate(losses.begin() + position,
        losses.begin() + position + num_pkts, 0)) /
        float(num_pkts) / float(num_occurrences_of_consecutive_losses);
  }
  return average_loss_frac;
}

tuple<float, float, float> LossMetricComputer::get_window_metrics(
      const LossInfo & lossInfo)
{
  uint16_t num_pkts = lossInfo.packet_losses.size();
  if (num_pkts == 0) { return make_tuple(0, 0, 0); }
  uint16_t num_missing_pkts = accumulate(lossInfo.packet_losses.begin(),
      lossInfo.packet_losses.end(), 0);
  auto mp = get_bursts_and_guardspaces(lossInfo.packet_losses);
  uint16_t max_burst_length = mp.first.size() > 0? mp.first.rbegin()->first : 0;
  return make_tuple(num_pkts, num_missing_pkts, max_burst_length);
}
