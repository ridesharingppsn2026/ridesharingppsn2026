#include "population_utils.h"
#include "../src/main.h"
#include "../grid/BoundedParetoSet.cpp"
#include "../src/nd-tree/ND-Tree.h"
#include "solution_utils.h"
#include "generic_algorithms.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>
#include <numeric>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Population creation and initialization
Population get_random_population(Instance& instance, BoundedParetoSet *EP, int max_population, int &valuations) {
  Population population;
  int i = 0;
  while(i < max_population) {
    Solution solution;
    get_random_solution(instance, solution);
    valuations++;
    if(solution_validity(instance, solution)){
      population.population.push_back(solution);
      i++;
      EP->adicionarSol(&solution);
    }
  }
  return population;
}

void get_random_population(Instance& instance, Population& population, BoundedParetoSet *EP, int max_population, int &valuations) {
  int i = 0;
  while(i < max_population) {
    Solution solution;
    get_random_solution(instance, solution);
    valuations++;
    if(solution_validity(instance, solution)){
      population.population.push_back(solution);
      i++;
      EP->adicionarSol(&solution);
    }
  }
}

// Population analysis
Population get_non_dominated_population(Population received_population){
  Population population = received_population;
  for(int solution = 0; solution < population.population.size(); solution++){
    for(int solution_to_compare = solution+1; solution_to_compare  < population.population.size(); solution_to_compare++){
      if(x_dominates_y(population.population[solution], population.population[solution_to_compare])){
        population.population.erase(population.population.begin() + solution_to_compare );
        solution_to_compare--;
      }
      else if(x_dominates_y(population.population[solution_to_compare], population.population[solution])){
        population.population.erase(population.population.begin() + solution );
        solution--;
        break;
      }
      else if(population.population[solution].cost == population.population[solution_to_compare].cost and population.population[solution].total_bonus == population.population[solution_to_compare].total_bonus and population.population[solution].time == population.population[solution_to_compare].time){
        population.population.erase(population.population.begin() + solution_to_compare );
        solution_to_compare--;
      }
    }
  }
  return population;
} //return only the non-dominated solutions

Pareto_objectives get_pareto_objectives(Population received_population){
  Population population = get_non_dominated_population(received_population);
  Pareto_objectives matrix_of_objectives;
  for(int solution = 0; solution < population.population.size(); solution++){
   Objectives vector_of_objectives;
   vector_of_objectives.cost = population.population[solution].cost;
   vector_of_objectives.time = population.population[solution].time;
   vector_of_objectives.total_bonus = population.population[solution].total_bonus;

   matrix_of_objectives.pareto_set.push_back(vector_of_objectives);
  }
  return matrix_of_objectives;
}// returns a Pareto_objectives variable with all objectives from the population

Pareto_objectives get_pareto_objectives(BoundedParetoSet *received_population){
  Pareto_objectives matrix_of_objectives;
  int pop_size = received_population->get_size();
  for(int solution = 0; solution < pop_size; solution++){
    matrix_of_objectives.pareto_set.push_back(received_population->get_objectives(solution));
    matrix_of_objectives.solutions.push_back(received_population->get_solution(solution));
  }
  return matrix_of_objectives;
}// returns a Pareto_objectives variable with all objectives from the population

Pareto_objectives get_pareto_objectives(NDTree& archive){
  Pareto_objectives matrix_of_objectives;
  vector<Solution> allSolutions;
  archive.root->GetAllSolutions(allSolutions);
  int pop_size = archive.current_size;
  for(auto& solution : allSolutions){
    Objectives objectives;
    objectives.cost = solution.cost;
    objectives.time = solution.time;
    objectives.total_bonus = solution.total_bonus;
    matrix_of_objectives.pareto_set.push_back(objectives);
    matrix_of_objectives.solutions.push_back(solution);
  }
  return matrix_of_objectives;
} // returns a Pareto_objectives variable with all objectives from the population

// Population mutation
void mutate_routes(Instance& instance, Population &population) {
  for (int i = 0; i < population.population.size(); i++) {
    mutate_routes(instance, population.population[i], 3);
  }
}
// mutates a population in three different ways: swapping, removing and adding a new vertex


void able_passengers(Instance& instance, Population &population) {
  for(int solution = 0; solution<population.population.size();solution++){
    able_passengers(instance, population.population[solution]);
  }
} // applies "able passenger"to all individuals in the population


// Population crossover
Solution one_crossover(const Instance& instance, vector<Solution> &population, int father, int mother) {
  Solution baby;
  for (int i = 0; i < population[father].route.size() / 2; i++) {
    baby.route.push_back(population[father].route[i]);
    baby.cities_colected.push_back( population[father].cities_colected[i]);
  } // first half of the route and collections from the father

  for (int j = population[mother].route.size() / 2;
       j < population[mother].route.size(); j++) {
    bool gene_is_repeated = false;
    for (int k = 0; k < population[father].route.size() / 2; k++) {
      if (population[mother].route[j] ==  population[father].route[k]) {
        gene_is_repeated = true;
      }
    }
    if (gene_is_repeated == false) {
      baby.route.push_back(population[mother].route[j]);
      baby.cities_colected.push_back(population[mother].cities_colected[j]);
    }
  } // second half of the route and collections from the mother
  return baby;
}// returns a child that will have the initial half of the parent's route, second half of the mother's route (except repeated cities), collecting bonuses if they collect in this city. 
// THIS FUNCTION DOES NOT CALL passenger loading heuristics

Solution one_crossover(Instance instance, vector<Solution> &population) {
  int father = rand() % population.size(), mother = rand() % population.size();
  Solution baby = one_crossover(instance, population, father, mother);
  return baby;
}

vector<Solution> crossover(Instance& instance, Population &parents) {
  vector<Solution> children;
  while (children.size() < parents.population.size()) {
    Solution baby = one_crossover(instance, parents.population);
    update_objectives(instance, baby);
    children.push_back(baby);
  }
  return children;
}
// returns a population of children equal to the size of the population of parents

vector<Solution> crossover_and_mutate(
  Generations& generations,
  Instance& instance, 
  BoundedParetoSet *EP, 
  Population &parents, 
  int max_valuations, 
  int crossover_chance, 
  int mutation_chance, 
  int& valuations, 
  int &biggest_multiple
) {
  vector<Solution> children;
  int valid_children_count =0;
  while (valid_children_count < parents.population.size()) {
    Solution baby;
    if (crossover_chance < random_chance()) {
      baby = one_crossover(instance, parents.population);
      able_passengers(instance, baby);
      update_objectives(instance, baby);
      if(solution_validity(instance, baby)){
        EP->adicionarSol(&baby);
      }
      valuations++;
      int multiple = biggest_multiple+1;
      if(multiple*10000<=valuations){
        //cout<<"saving data"<<endl;
        save_data_routine(generations,EP, biggest_multiple, valuations);
        if(valuations>=max_valuations){
          return children;
        }
        biggest_multiple++;
      }
      if (mutation_chance < random_chance()) {
        mutate_routes(instance, baby, 3);
        mutate_bonuses(instance, baby);
        able_passengers(instance, baby);
        update_objectives(instance, baby);
        if(solution_validity(instance, baby)){
          EP->adicionarSol(&baby);
        }
        valuations++;
        int multiple = biggest_multiple+1;
        if(multiple*10000<=valuations){
          //cout<<"saving data"<<endl;
          save_data_routine(generations,EP, biggest_multiple, valuations);
          if(valuations>=max_valuations){
            return children;
          }
          biggest_multiple++;
        }
      }
    } else {
      baby = get_random_solution(instance); 
      if(solution_validity(instance, baby)){
        EP->adicionarSol(&baby);
      }
      valuations++;
      int multiple = biggest_multiple+1;
      if(multiple*10000<=valuations){
        //cout<<"saving data"<<endl;
        save_data_routine(generations,EP, biggest_multiple, valuations);
        if(valuations>=max_valuations){
          return children;
        }
        biggest_multiple++;
      }
    }
    if(solution_validity(instance, baby)){
      valid_children_count++;
      bool solution_repeated = false;
      for(int i =0; i<parents.population.size();i++){
        if(baby.cost == parents.population[i].cost and baby.time == parents.population[i].time and baby.total_bonus == parents.population[i].total_bonus){
          solution_repeated = true;
        }
      }
      if(solution_repeated == false ){
        children.push_back(baby);
      }
    }

  }
  return children;
}

vector<Solution> crossover_and_mutate(
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
  duration<double>& tempoTotal
) {
  vector<Solution> children;
  int valid_children_count =0;
  while (valid_children_count < parents.population.size()) {
    Solution baby;
    if (crossover_chance < random_chance()) {
      baby = one_crossover(instance, parents.population);
      able_passengers(instance, baby);
      update_objectives(instance, baby);
      if(solution_validity(instance, baby)){
        EP->adicionarSol(&baby);
      }
      valuations++;
      int multiple = biggest_multiple+1;
      if(multiple*10000<=valuations){
        //cout<<"saving data"<<endl;
        save_data_routine(generations, EP, inicio, biggest_multiple, valuations, tempoTotal);
        if(valuations>=max_valuations){
          return children;
        }
        biggest_multiple++;
      }
      if (mutation_chance < random_chance()) {
        mutate_routes(instance, baby, 3);
        mutate_bonuses(instance, baby);
        able_passengers(instance, baby);
        update_objectives(instance, baby);
        if(solution_validity(instance, baby)){
          EP->adicionarSol(&baby);
        }
        valuations++;
        int multiple = biggest_multiple+1;
        if(multiple*10000<=valuations){
          //cout<<"saving data"<<endl;
          save_data_routine(generations, EP, inicio, biggest_multiple, valuations, tempoTotal);
          if(valuations>=max_valuations){
            return children;
          }
          biggest_multiple++;
        }
      }
    } else {
      baby = get_random_solution(instance); 
      if(solution_validity(instance, baby)){
        EP->adicionarSol(&baby);
      }
      valuations++;
      int multiple = biggest_multiple+1;
      if(multiple*10000<=valuations){
        //cout<<"saving data"<<endl;
        save_data_routine(generations, EP, inicio, biggest_multiple, valuations, tempoTotal);
        if(valuations>=max_valuations){
          return children;
        }
        biggest_multiple++;
      }
    }
    if(solution_validity(instance, baby)){
      valid_children_count++;
      bool solution_repeated = false;
      for(int i =0; i<parents.population.size();i++){
        if(baby.cost == parents.population[i].cost and baby.time == parents.population[i].time and baby.total_bonus == parents.population[i].total_bonus){
          solution_repeated = true;
        }
      }
      if(solution_repeated == false ){
        children.push_back(baby);
      }
    }

  }
  return children;
}

// Data saving
void save_data_routine(Generations& generations, BoundedParetoSet *EP, int &biggest_multiple, int valuations){
    Pareto_objectives sets_of_objectives = get_pareto_objectives(EP);
    generations.generations.push_back(sets_of_objectives);
}

void save_data_routine(Generations& generations, BoundedParetoSet *EP, std::chrono::high_resolution_clock::time_point &inicio, int &biggest_multiple, int valuations, duration<double>& tempoTotal){
  auto fim = high_resolution_clock::now();
  duration<double> tempotrecho = duration_cast<duration<double>>(fim - inicio);//update stretch time
  generations.durations.push_back(tempotrecho);
  tempoTotal += duration_cast<duration<double>>(fim - inicio); //adding total time
  Pareto_objectives sets_of_objectives = get_pareto_objectives(EP);
  generations.generations.push_back(sets_of_objectives);
  inicio = high_resolution_clock::now();
}

void save_data_routine(Generations& generations, NDTree& archive, std::chrono::high_resolution_clock::time_point &inicio, int &biggest_multiple, int valuations, duration<double>& tempoTotal){
  auto fim = high_resolution_clock::now();
  duration<double> tempotrecho = duration_cast<duration<double>>(fim - inicio);//update stretch time
  generations.durations.push_back(tempotrecho);
  tempoTotal += duration_cast<duration<double>>(fim - inicio); //adding total time
  Pareto_objectives sets_of_objectives = get_pareto_objectives(archive);
  generations.generations.push_back(sets_of_objectives);
  inicio = high_resolution_clock::now();
}
