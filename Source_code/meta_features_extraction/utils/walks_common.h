#ifndef WALKS_COMMON_H
#define WALKS_COMMON_H

#include "../src/main.h"
#include "solution_utils.h"

// Forward declarations
struct Instance;
struct Solution;

// Function declarations for walk operations
Solution get_random_neighbor(Instance& instance, const Solution& current);
Solution get_random_neighbor_pool(Instance& instance, const Solution& current);
std::vector<Solution> generate_all_neighbors(Instance& instance, const Solution& current);
std::vector<Solution> random_walk(Instance& instance, int length, int rate);
std::vector<Solution> adaptive_walk_first_improvement(Instance& instance, int breaking_point, int ratio);
std::vector<Solution> adaptive_walk_custoso(Instance& instance, const Solution& initial, int breaking_point);

#endif
