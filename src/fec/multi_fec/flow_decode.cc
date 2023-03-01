#include "flow_decode.hh"

#include <stdio.h>

using namespace std;

vector<int> filter_unusable(vector<uint16_t> counts,
    vector<bool> available_positions)
{
  vector<int> available_counts(available_positions.size(), 0);
  for (uint16_t pos = 0; pos < available_positions.size(); pos++) {
    if (available_positions.at(pos)) {
      available_counts.at(pos) = int(counts.at(pos));
    }
  }
  return available_counts;
}

Graph_III construct_graph(vector<uint16_t> num_missing_us, vector<uint16_t>
    num_missing_vs, vector<uint16_t> num_received_parities, vector<bool>
    recovered_us, vector<bool> recovered_vs, vector<bool> unusable_parities)
{
  vector<int> num_recovered_us = filter_unusable(num_missing_us,
      recovered_us);
  vector<int> num_recovered_vs = filter_unusable(num_missing_vs,
      recovered_vs);
  vector<bool> usable_parities;
  for (auto el : unusable_parities) { usable_parities.push_back(!el); }
  vector<int> num_use_parities = filter_unusable(num_received_parities,
      usable_parities);
  return construct_graph(num_recovered_us, num_recovered_vs, num_use_parities);
}

Graph_III construct_graph(std::vector<int> num_recovered_us, std::vector<int>
    num_recovered_vs, std::vector<int> num_usable_parities)
{
  int num_frames = num_recovered_us.size();
  assert(num_frames == num_recovered_vs.size());
  assert(num_frames == num_usable_parities.size());
  int delay = num_frames / 2;
  int num_vertices = 3 * num_frames - delay;
  int num_use_parities = num_frames - delay;
  Graph_III graph(num_vertices, num_use_parities + (num_use_parities) *
      (delay + 3) + 2 * num_frames);
  for (uint16_t v = 0; v < uint16_t(num_vertices); v++) {
    graph.add_node();
  }
  for (uint16_t parity = 0; parity < uint16_t(num_use_parities); parity++) {
    int capacity = num_usable_parities.at(delay + parity);
    graph.add_tweights(int(parity), capacity, 0);
    for (uint16_t v = parity; v <= parity + delay; v++) {
      graph.add_edge(int(parity), num_use_parities + int(v), capacity, 0);
    }
    graph.add_edge(int(parity), num_use_parities + num_frames + int(parity),
        capacity, 0);
    graph.add_edge(int(parity), num_use_parities + num_frames + int(parity +
        delay), capacity, 0);
  }
  uint16_t final_v = num_use_parities + num_frames - 1;
  for (uint16_t pos = uint16_t(num_use_parities); pos <= final_v ; pos++) {
    graph.add_tweights(int(pos), 0, num_recovered_vs.at(pos -
        uint16_t(num_use_parities)));
  }
  uint16_t final_u = final_v + num_frames;
  for (uint16_t pos = final_v + 1; pos <= final_u; pos++) {
    graph.add_tweights(int(pos), 0, num_recovered_us.at(pos - final_v - 1));
  }
  return graph;
}

vector<uint16_t> get_used_parities(Graph_III & graph, vector<int>
    num_usable_parities, uint16_t num_frames) {
  vector<uint16_t> parities_used(num_frames, 0);
  uint16_t delay = num_frames / 2;
  graph.maxflow();
  for (uint16_t frame_num = 0; frame_num < delay + 1; frame_num++) {
    parities_used.at(frame_num + delay) = uint16_t(
      num_usable_parities.at(frame_num + delay)) - uint16_t(graph.get_trcap(
      frame_num));
  }
  return parities_used;
}

vector<uint16_t> get_used_parity_counts(vector<uint16_t> num_missing_us,
    vector<uint16_t> num_missing_vs, vector<uint16_t>
    num_received_parities, vector<bool> recovered_us,
    vector<bool> recovered_vs, vector<bool> unusable_parities)
{
  assert(num_missing_us.size() == num_missing_vs.size());
  assert(num_missing_vs.size() == num_received_parities.size());
  assert(num_received_parities.size() == recovered_us.size());
  assert(recovered_us.size() == recovered_us.size());
  assert(recovered_us.size() == unusable_parities.size());
  auto graph = construct_graph(num_missing_us, num_missing_vs,
      num_received_parities, recovered_us, recovered_vs, unusable_parities);
  vector<bool> usable_parities;
  for (auto el : unusable_parities) { usable_parities.push_back(!el); }
  return get_used_parities(graph, filter_unusable(num_received_parities,
      usable_parities), recovered_us.size());
}
