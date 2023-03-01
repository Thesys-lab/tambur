#ifndef FLOW_DECODE_HH
#define FLOW_DECODE_HH

#include <cmath>
#include <limits.h>
#include <string.h>
#include <queue>
#include <stdint.h>

#include <maxflow.h>

using std::pair;
using std::make_pair;
using maxflow::Graph_III;

std::vector<int> filter_unusable(std::vector<uint16_t> counts,
    std::vector<bool> available_positions);

Graph_III construct_graph(std::vector<uint16_t> num_missing_us,
    std::vector<uint16_t> num_missing_vs, std::vector<uint16_t>
    num_received_parities, std::vector<bool> recovered_us,
    std::vector<bool> recovered_vs, std::vector<bool> unusable_parities);

Graph_III construct_graph(std::vector<int> num_recovered_us,
    std::vector<int> num_recovered_vs, std::vector<int> num_usable_parities);

std::vector<uint16_t> get_used_parities(Graph_III & graph, std::vector<int>
    num_usable_parities, uint16_t num_frames);

std::vector<uint16_t> get_used_parity_counts(std::vector<uint16_t>
    num_missing_us, std::vector<uint16_t> num_missing_vs, std::vector<uint16_t>
    num_received_parities, std::vector<bool> recovered_us,
    std::vector<bool> recovered_vs, std::vector<bool> unusable_parities);

#endif /* FLOW_DECODE_HH */
