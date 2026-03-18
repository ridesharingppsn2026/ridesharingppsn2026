#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>
#include <numeric>

// Forward declarations
struct Instance;
struct Solution;
struct Passenger;

// Solution creation and initialization
Solution get_random_solution(Instance& instance);
void get_random_solution(Instance& instance, Solution& solution);
std::vector<int> get_random_route(Instance& instance);
void get_random_route(Instance& instance, Solution& solution);
std::vector<bool> get_random_bonus(Instance& instance, int route_size);
void get_random_bonus(Instance& instance, Solution& solution);

// Solution validation
bool solution_validity(Instance& instance, Solution& solution);
bool check_passengers_riding(const Instance& instance, const Solution& solution);

// Solution properties calculation
int get_bonus(const Instance& instance, Solution& solution);
int get_time(const Instance& instance, Solution& solution);
double get_cost(const Instance& instance, Solution& solution);
void update_objectives(const Instance& instance, Solution& solution);
double getObj(const Solution& s, int k);

// Passenger management
void able_passengers(const Instance& instance, Solution& solution);
double calculate_passenger_cost(int origem_index, int destiny_index, std::vector<int>& passengers_in_car_by_city, const Solution& solution, const Instance& instance);
double calculate_passenger_time(int origem_index, int destiny_index, const Solution& solution, const Instance& instance);

// Solution mutation
void mutate_routes(const Instance& instance, Solution& solution, int mode);
void mutate_bonuses(const Instance& instance, Solution& solution);
void swap_random_route_slots(Solution& solution);

// Solution comparison
bool x_dominates_y(const Solution& solutionx, const Solution& solutiony);
bool dominates(const Solution& solutionx, const Solution& solutiony); //same thing
double euclidean_distance(const Solution& a, const Solution& b);

// Utility functions
int random_city(Instance instance);
int random_chance();
