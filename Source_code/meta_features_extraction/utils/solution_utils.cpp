#include "solution_utils.h"
#include "../src/main.h"
#include "generic_algorithms.h"
#include "io_utils.h"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>
#include <numeric>

using namespace std;

// Solution creation and initialization
Solution get_random_solution(Instance& instance) {
  Solution solution;
  get_random_route(instance,solution);
  get_random_bonus(instance, solution);
  able_passengers(instance, solution);
  update_objectives(instance, solution);
  return solution;
}

void get_random_solution(Instance& instance, Solution& solution) {
  get_random_route(instance, solution);
  get_random_bonus(instance, solution);
  able_passengers(instance, solution);
  update_objectives(instance, solution);
}

vector<int> get_random_route(Instance& instance) {
  int number_of_cities = 0;
  while (number_of_cities < 2) {
    number_of_cities = rand() % instance.number_of_cities;
  } // define a number of cities >= 2
  vector<int> route;
  vector<bool> visited_cities(instance.number_of_cities, false);
  route.push_back(0); // satisfy origin always 0
  visited_cities[0] = 0; 
  int i = 1;
  while (i < number_of_cities) {
    int city = random_city(instance);
    if (visited_cities[city] != true) {
      route.push_back(city);
      visited_cities[city] = true;
      i++;
    }
  }
  return route;
}

void get_random_route(Instance& instance, Solution& solution) {
    solution.route.clear();
    
    // ||Generate number of cities (2 <= n <= instance.number_of_cities)
    int number_of_cities = 2 + (rand() % (instance.number_of_cities - 1));
    
    // ||Temporary vector with all cities except the origin (0)
    vector<int> cities(instance.number_of_cities - 1);
    for(int i = 1; i < instance.number_of_cities; ++i) {
        cities[i-1] = i;
    }
    
    // Shuffle using Fisher-Yates
    for(int i = cities.size() - 1; i > 0; --i) {
        int j = rand() % (i + 1);
        swap(cities[i], cities[j]);
    }
    
    // Build the route
    solution.route.push_back(0); // origin
    for(int i = 0; i < number_of_cities - 1; ++i) {
        solution.route.push_back(cities[i]);
    }
}// number of cities will be at minimum 2, max all cities, equal chances to all possibilities

vector<bool> get_random_bonus(Instance& instance, int route_size) {
  vector<bool> cities_colected;
  cities_colected.push_back(false); //first city is not collected
  for (int i = 0; i < route_size-1; i++) {
    if (rand() % 2 == 0) {
      cities_colected.push_back(true);
    } else {
      cities_colected.push_back(false);
    }
  }
  return cities_colected;
}
// 50% chance to collect a bonus. Treats cities_collected as having the same size as route. 
// This implies that the city in route[x] was collected if cities_collected[x] is True.


void get_random_bonus(Instance& instance, Solution& solution) {
  solution.cities_colected.push_back(false); //first city is not collected
  for (int i = 0; i < solution.route.size()-1; i++) {
    if (rand() % 2 == 0) {
      solution.cities_colected.push_back(true);
    } else {
      solution.cities_colected.push_back(false);
    }
  }
}

// Solution validation
bool solution_validity(Instance& instance, Solution& solution){
  bool validity = true;
  if(solution.route.size()<2){
    validity = false;
    cout<<"rota menor que 2"<<endl;
  }
  if(solution.route[0]!=0){
      validity = false;
      cout<<"Origem não é 0"<<endl;
  }
  if(solution.cost==0 or solution.time ==0){
    validity = false;
    cout<<"custo ou tempo 0"<<endl;
  }
  if(solution.cost != get_cost(instance, solution)){
    validity = false;
    cout<<"custo errado"<<endl;
  }
  if(solution.time != get_time(instance, solution)){
    validity = false;
    cout<<"tempo errado"<<endl;
  }
  if(solution.total_bonus != get_bonus(instance, solution)){
    validity = false;
    cout<<"bonus errado"<<endl;
  }
  if(check_passengers_riding(instance, solution)==false){
    validity = false;
    cout<<"passageiros errado"<<endl;
  }
    if(validity ==false){cout<<"erro:"<<endl; print_solution(instance,solution);}

  return validity;
}

bool check_passengers_riding(const Instance& instance, const Solution& solution) {
    vector<int> passengers_in_car_by_city(solution.route.size(), 1); 
    vector<pair<double, int>> passengers_by_cost;
    for(int i =0; i <instance.number_of_passengers;i++){//adds all passengers loaded from the solution to cost and index pairs
      if(solution.passengers_riding[i]){ 
        passengers_by_cost.push_back(make_pair(instance.passengers[i].max_cost,i));
      }
    }
    if(passengers_by_cost.size()==0){
      return true;
    }
    heapSort(passengers_by_cost); //here we have already ordered the passengers by poorest

    for(int passenger = passengers_by_cost.size() - 1; passenger >= 0; passenger--) { // Goes from richest to poorest
        int origem_index = -1, destiny_index = -1;
        double cost = 0.0;
        int time = 0;
        bool subiu = false, desceu = false, apto = true;

        for (int city = 0; city < solution.route.size(); city++) {
            if (solution.route[city] == instance.passengers[passengers_by_cost[passenger].second].origin) {
                // Passenger can board
                if (passengers_in_car_by_city[city] < instance.car_capacity) {
                    origem_index = city;
                    subiu = true;
                } else {
                    return false;
                }
            }else if(solution.route[city] == instance.passengers[passengers_by_cost[passenger].second].destiny) {
                // Passenger gets out
                if(city==0){ //special case in which destination is starting city
                    desceu = true;
                    destiny_index = city;
                }
                else{
                  if (subiu) { 
                      destiny_index = city;
                      desceu = true;
                      break;
                  } else {
                      return false;
                  }
                }
            }else if (subiu) {
                if (passengers_in_car_by_city[city] >= instance.car_capacity) {// if car is full
                  return false;
                  break;
                }
            }
        }

        // Handles the case where the passenger's destination is the starting city (route that returns to the point of origin)
        if (subiu && desceu && solution.route[0] == instance.passengers[passengers_by_cost[passenger].second].destiny) {
            if(passengers_in_car_by_city[solution.route.size()-1] >= instance.car_capacity){ //tests if the car is full
              return false;
            }
        }

        // OBJECTIVE TESTING PHASE: If the passenger went up and down, calculate the cost and time and test if he can afford it
        if (subiu && desceu && apto) {
            cost = calculate_passenger_cost(origem_index, destiny_index, passengers_in_car_by_city, solution, instance);
            time = calculate_passenger_time(origem_index, destiny_index, solution, instance);

            // Checks if the passenger can afford the cost and time
            if (cost > instance.passengers[passengers_by_cost[passenger].second].max_cost || 
                time > instance.passengers[passengers_by_cost[passenger].second].max_time) {
                return false;
            }
        }

        // POST-TESTING PHASE: If the passenger is able, mark the occupancy of the car in the sections where it was boarded
        if (subiu && desceu && apto) {
            if (destiny_index == 0) {
                passengers_in_car_by_city[solution.route.size()-1]++;
                destiny_index = solution.route.size() - 1;
            }
            for (int city = origem_index; city < destiny_index; city++){
                passengers_in_car_by_city[city]++;
            }
        }
    }

    return true;
}

// Solution properties calculation
int get_bonus(const Instance& instance, Solution &solution) {
  int bonus = 0;
  for (int city = 0; city < solution.route.size(); city++) {
    if (solution.cities_colected[city]) { // add bonus from cities collected
      bonus += instance.bonus_and_time[solution.route[city]].first;
    }
  }
  return bonus;
}

int get_time(const Instance& instance, Solution &solution) {
  int time = 0;
  for (int city = 0; city < solution.route.size(); city++) {
    int next_city;
    if(city==solution.route.size()-1){
      next_city=0;
    }
    else{
      next_city=city+1;
    }
    time += instance.time_matrix[solution.route[city]][solution.route[next_city]];

    if (solution.cities_colected[city]){ // add time from bonus collecting
      time += instance.bonus_and_time[solution.route[city]].second;
    }
  } 
  return time;
}
// calculates and returns the total time of the solution

double get_cost(const Instance& instance, Solution &solution) {
  double cost = 0;
  int people_in_car = 1;

  for (int city = 0; city < solution.route.size(); city++) {
    double temp_cost = 0;
    int next_city;
    if(city==solution.route.size()-1){
      next_city=0;
    }
    else{
      next_city=city+1;
    }
    temp_cost = instance.cost_matrix[solution.route[city]][solution.route[next_city]];
    if(solution.passengers_riding.size()>0){
      for (int passenger = 0; passenger < instance.number_of_passengers; passenger++) {
        if (solution.passengers_riding[passenger]==true) {
          if (instance.passengers[passenger].origin == solution.route[city]) {
            people_in_car++;
          }
          if (instance.passengers[passenger].destiny == solution.route[city]) {
            if(city !=0){
              people_in_car--;
            }
          }
        }
      }
    }
    temp_cost /= people_in_car;
    cost += temp_cost;
  } // at every city checks how many are in the car to calculate the cost for
    // this part of the route
  return cost;
}

void update_objectives(const Instance& instance, Solution &solution) {
  solution.time = get_time(instance, solution);
  solution.cost = get_cost(instance, solution);
  solution.total_bonus = get_bonus(instance, solution);
}
// calls the 3 functions to query the value of the objectives and updates them in the respective places


double getObj(const Solution& s, int k){
	if(k == 0){
		return s.cost;
	}
	else if(k==1){
		return s.time;
	}
	else if(k==2){
		return s.total_bonus;
	}
  else{
    return -1;
  }
}

// Passenger management
void able_passengers(const Instance& instance, Solution &solution) {
    vector<bool> able_passengers(instance.number_of_passengers, false); 
    vector<int> passengers_in_car_by_city(solution.route.size(), 1); 
    vector<pair<double, int>> passengers_by_cost;

    for(int i = 0; i < instance.number_of_passengers; i++) {
        passengers_by_cost.push_back(make_pair(instance.passengers[i].max_cost, i));
    }

    heapSort(passengers_by_cost);  // Sort passengers by cost (from poorest to richest)

    for(int passenger = passengers_by_cost.size() - 1; passenger >= 0; passenger--) {  // Goes from richest to poorest
        int origem_index = -1, destiny_index = -1;
        double cost = 0.0;
        double time = 0;
        bool subiu = false, desceu = false, apto = true;

        for (int city = 0; city < solution.route.size(); city++) {
            if (solution.route[city] == instance.passengers[passengers_by_cost[passenger].second].origin) {
                // Passenger can board
                if (passengers_in_car_by_city[city] < instance.car_capacity) {
                    origem_index = city;
                    subiu = true;
                } else {
                    apto = false;
                    break;
                }
            } else if(solution.route[city] == instance.passengers[passengers_by_cost[passenger].second].destiny) {
                // Passenger disembarks
                if(city==0){ //special case in which destination is starting city
                    desceu = true;
                    destiny_index = city;
                }
                else{
                  if (subiu) { 
                      destiny_index = city;
                      desceu = true;
                      break;
                  } else {
                      apto = false;
                      break;
                  }
                }
            } else if (subiu) {
                if (passengers_in_car_by_city[city] >= instance.car_capacity) {// If the car is full
                  apto = false;
                  break;
                }
            }
        }

        // Handles the case where the passenger's destination is the starting city (route that returns to the point of origin)
        if (subiu && desceu && solution.route[0] == instance.passengers[passengers_by_cost[passenger].second].destiny) {
            if(passengers_in_car_by_city[solution.route.size()-1] >= instance.car_capacity){ //||check if the car is full
              apto = false;
            }
        }

        // OBJECTIVE TESTING PHASE: If the passenger went up and down, calculates the cost, time and tests if he can afford it
        if (subiu && desceu && apto) {
            cost = calculate_passenger_cost(origem_index, destiny_index, passengers_in_car_by_city, solution, instance);
            time = calculate_passenger_time(origem_index, destiny_index, solution, instance);

            // Check whether the passenger can afford the cost and time
            if (cost > instance.passengers[passengers_by_cost[passenger].second].max_cost || 
                time > instance.passengers[passengers_by_cost[passenger].second].max_time) {
                apto = false;
            }
        }

        // POST-TESTING PHASE: If the passenger is able, mark it as "able" and mark the car's occupancy in the sections where he is boarded
        if (subiu && desceu && apto) {
            able_passengers[passengers_by_cost[passenger].second] = true;
            if (destiny_index == 0) {
                passengers_in_car_by_city[solution.route.size()-1]++;
                destiny_index = solution.route.size() - 1;
            }
            for (int city = origem_index; city < destiny_index; city++){
                passengers_in_car_by_city[city]++;
            }
        }
    }
    solution.passengers_riding = able_passengers;
}

double calculate_passenger_cost(int origem_index, int destiny_index, vector<int>& passengers_in_car_by_city, const Solution& solution, const Instance& instance) {
    double cost = 0.0;
    // If the destination is the starting city, include the cost of the last part
    if (destiny_index == 0) {
        cost += instance.cost_matrix[solution.route[solution.route.size() - 1]][solution.route[0]] / (1+passengers_in_car_by_city[solution.route.size() - 1]);
        destiny_index = solution.route.size() - 1;
    }
    for (int i = origem_index; i < destiny_index; i++) {
        int next_index = i + 1;
        cost += instance.cost_matrix[solution.route[i]][solution.route[next_index]] / (1+ passengers_in_car_by_city[i]);
    }
    return cost;
}

// Function to calculate passenger time
double calculate_passenger_time(int origem_index, int destiny_index, const Solution& solution, const Instance& instance) {
    double time = 0;
    // If the destination is the starting city, include the time of the last part  
    if (destiny_index == 0) {
        time += instance.time_matrix[solution.route[solution.route.size() - 1]][solution.route[0]];
        destiny_index = solution.route.size() - 1;
    }
    for (int i = origem_index; i < destiny_index; i++) {
        int next_index = i + 1;
        time += instance.time_matrix[solution.route[i]][solution.route[next_index]];
        if (solution.cities_colected[i]) { //boarding and disembarking happens after bonus collection
          time += instance.bonus_and_time[solution.route[i]].second;
        }
    }
    if (solution.cities_colected[destiny_index]) { //boarding and disembarking happens after bonus collection
          time += instance.bonus_and_time[solution.route[destiny_index]].second;
    }
    return time;
}

// Solution mutation
void mutate_routes(const Instance& instance, Solution &Solution, int mode) {
  if (mode == 3) {
    if (Solution.route.size() <= 2) {
      mode = 2;
    } else if (Solution.route.size() == instance.number_of_cities) {
      mode = rand() % 2;
    } else {
      mode = rand() % 3;
    }
  } // this if ensures that if mutate_routes is called, it will definitely occur a correct mutation. 
  if (mode == 0 and Solution.route.size()>2) { // swap vertex (but not origin)
    int city1 = 0;
    while(city1 == 0){
      city1 = rand() % Solution.route.size();
    } 
    int city2 = 0;
    while(city2==0 or city2==city1){
      city2 = rand() % Solution.route.size();
    }
    swap(Solution.route[city1], Solution.route[city2]);
    swap(Solution.cities_colected[city1], Solution.cities_colected[city2]);
  } 
  else if (mode == 1 and Solution.route.size() > 2) { // remove random vertex, but not origin
      int city_to_diminish = 0;
      while(city_to_diminish==0){
        city_to_diminish = rand() % Solution.route.size();
      }
      Solution.route.erase(Solution.route.begin() + city_to_diminish);
      Solution.cities_colected.erase(Solution.cities_colected.begin() + city_to_diminish);
  }
  else if (mode == 2 and Solution.route.size() < instance.number_of_cities) { 
    // add random vertex that is not already in the route in any place except origin
  
      vector<int> cities_not_in_route;
      bool marker = false;
      for (int j = 0; j < instance.number_of_cities; j++) {
        marker = false;
        for (int k = 0; k < Solution.route.size(); k++) {
          if (Solution.route[k] == j) {
            marker = true;
            break;
          }
        }
        if (marker == false) {
          cities_not_in_route.push_back(j);
        }
      }
      int city_to_add = rand() % cities_not_in_route.size();
      int city_to_add_index = rand() % Solution.route.size();
      while(city_to_add_index==0){
        city_to_add_index = rand() % Solution.route.size();
      }
      Solution.route.insert(Solution.route.begin() + city_to_add_index, cities_not_in_route[city_to_add]);
      Solution.cities_colected.insert(Solution.cities_colected.begin() + city_to_add_index, false);
  }
}
// mutates a solution in four different settings according to "mode": 0- swapping, 1 - removing, 2 - adding a new vertex and 3 - random

void mutate_bonuses(const Instance& instance, Solution &solution) {
  int bonus_change = rand() % solution.cities_colected.size();
  while(bonus_change==0){
    bonus_change = rand() % solution.cities_colected.size();
  }
  if (solution.cities_colected[bonus_change] == false) {
    solution.cities_colected[bonus_change] = true;
  } else {
    solution.cities_colected[bonus_change] = false;
  }
} //mutates a solution by inverting a bit in bonus colecting vector


void swap_random_route_slots(Solution &solution) {
  int i = rand() % solution.route.size();
  int j = rand() % solution.route.size();
  swap(solution.route[i], solution.route[j]);
}

// Solution comparison
bool x_dominates_y(const Solution& solutionx, const Solution& solutiony) {
  if (solutionx.cost <= solutiony.cost && solutionx.time <= solutiony.time &&
      solutionx.total_bonus >= solutiony.total_bonus) {
    if (solutionx.cost < solutiony.cost || solutionx.time < solutiony.time ||
        solutionx.total_bonus > solutiony.total_bonus) {
      return true;
    }
  }
  return false;
} //Returns true only if solutionx dominates solutiony

// Solution comparison
bool dominates(const Solution& solutionx, const Solution& solutiony) {
  if (solutionx.cost <= solutiony.cost && solutionx.time <= solutiony.time &&
      solutionx.total_bonus >= solutiony.total_bonus) {
    if (solutionx.cost < solutiony.cost || solutionx.time < solutiony.time ||
        solutionx.total_bonus > solutiony.total_bonus) {
      return true;
    }
  }
  return false;
} //Returns true only if solutionx dominates solutiony

double euclidean_distance(const Solution& a, const Solution& b) {
  return sqrt(pow(a.cost - b.cost, 2) + pow(a.time - b.time, 2) + pow(a.total_bonus - b.total_bonus, 2));
}

// Utility functions
int random_city(Instance instance) {
  return rand() % instance.number_of_cities;
}

int random_chance() {
  int random_chance = rand() % 100;
  return random_chance;
}
