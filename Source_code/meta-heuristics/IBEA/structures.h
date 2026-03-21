#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <vector>
#include <list>
#include <chrono>

// ==================== data structures ====================
struct Passenger
{
    int wk; // ||maximum cost willing to pay
    int ok; // origin
    int dk; // destination
    int tk; // ||maximum time willing to accept
};

struct Solution
{
    double cost, time, total_bonus;
    std::vector<int> route;
    std::vector<bool> cities_colected;
    std::vector<bool> passengers_riding;
    double fitness;
};

// ||Structure to store solution sets per generation
struct Pareto_objectives {
    std::vector<Solution> solutions;
    std::vector<Solution> pareto_set;
};

struct Generations {
    std::vector<Pareto_objectives> generations;
    std::vector<std::chrono::duration<double>> durations;
};

// ||Helper functions for Solution manipulation
double getObj(const Solution& s, int k);
bool x_dominates_y(const Solution& x, const Solution& y);
bool solutions_are_equal(const Solution& a, const Solution& b);
double euclidean_distance(const Solution& a, const Solution& b);
double get_tchebycheff(const Solution& solution, std::vector<float> z, std::vector<double> weight_vector);
void print_solution(const Solution& solution);
bool solution_validity(const Solution& solution);

#endif // STRUCTURES_H 