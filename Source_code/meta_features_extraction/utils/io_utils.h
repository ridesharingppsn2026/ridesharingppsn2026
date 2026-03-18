#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>

// Forward declarations
struct Instance;
struct Solution;
struct Population;
struct Generations;

// Input functions
Instance get_instance(std::string file_path);
std::vector<std::string> generate_instance_paths();

// Output functions
void print_solution_full(Instance instance, Solution &solution);
void print_solution(Solution solution);
void print_solution(Instance instance, Solution &solution);
void print_population(Instance instance, Population population);
void shorter_print_population(Instance instance, Population population);
void print_instance(Instance& instance);
void print_fronts(Population population);

// File and directory management
void create_all_directories(int num_tests, int num_instances, std::string folder_name, std::vector<std::string> instances_addresses);
void create_test_archives(
  const Instance& instance, 
  std::string algorithm_name, 
  std::string folder_name, 
  std::string instance_address, 
  int current_test, 
  Generations& saved_generations, 
  unsigned int seed, 
  std::chrono::duration<double> tempoTotal
);
void fill_complete_archive(
  const Instance& instance, 
  std::ofstream &arquivo_completo, 
  Generations& saved_generations, 
  int generation, 
  int pareto_set
);
