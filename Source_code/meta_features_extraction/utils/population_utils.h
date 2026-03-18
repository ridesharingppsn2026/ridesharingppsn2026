#pragma once

#include <iostream>

#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>
#include <numeric>
#include "../src/main.h"

// Forward declarations
struct Instance;
struct Solution;
struct Population;
struct Pareto_objectives;
struct BoundedParetoSet;
struct NDTree;

// Population creation and initialization
Population get_random_population(Instance& instance, BoundedParetoSet *EP, int max_population, int &valuations);
void get_random_population(Instance& instance, Population& population, BoundedParetoSet *EP, int max_population, int &valuations);

// Population analysis
Population get_non_dominated_population(Population received_population);
Pareto_objectives get_pareto_objectives(Population received_population);
Pareto_objectives get_pareto_objectives(BoundedParetoSet *received_population);
Pareto_objectives get_pareto_objectives(NDTree& archive);

// Population mutation
void mutate_routes(Instance& instance, Population &population);
void able_passengers(Instance& instance, Population &population);

// Population crossover
Solution one_crossover(const Instance& instance, std::vector<Solution> &population, int father, int mother);
Solution one_crossover(Instance instance, std::vector<Solution> &population);
std::vector<Solution> crossover(Instance& instance, Population &parents);
std::vector<Solution> crossover_and_mutate(
  Generations& generations,
  Instance& instance, 
  BoundedParetoSet *EP, 
  Population &parents, 
  int max_valuations, 
  int crossover_chance, 
  int mutation_chance, 
  int& valuations, 
  int &biggest_multiple
);
std::vector<Solution> crossover_and_mutate(
  Generations& generations,
  Instance& instance, 
  BoundedParetoSet *EP, 
  Population &parents, 
  int max_valuations, 
  int crossover_chance, 
  int mutation_chance, 
  int& valuations, 
  int &biggest_multiple,
  std::chrono::high_resolution_clock::time_point &inicio,
  std::chrono::duration<double>& tempoTotal
);

// Data saving
void save_data_routine(Generations& generations, BoundedParetoSet *EP, int &biggest_multiple, int valuations);
void save_data_routine(Generations& generations, BoundedParetoSet *EP, std::chrono::high_resolution_clock::time_point &inicio, int &biggest_multiple, int valuations, std::chrono::duration<double>& tempoTotal);
void save_data_routine(Generations& generations, NDTree& archive, std::chrono::high_resolution_clock::time_point &inicio, int &biggest_multiple, int valuations, std::chrono::duration<double>& tempoTotal);
