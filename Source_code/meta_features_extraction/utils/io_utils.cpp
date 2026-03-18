#include "io_utils.h"
#include "../src/main.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <filesystem>
#include <chrono>

using namespace std;
using namespace std::chrono;

// Input functions
Instance get_instance(string file_path) {
  Instance instance;

  ifstream file(file_path.c_str());
  if (!file.is_open()) {
    cerr << "Erro ao abrir o arquivo " << file_path << endl;
    return instance;
  }

  // Reading data archives
  file >> instance.number_of_cities >> instance.number_of_passengers >> instance.car_capacity;

  // resizing matrices
  instance.cost_matrix.resize(instance.number_of_cities, vector<int>(instance.number_of_cities));
  instance.time_matrix.resize(instance.number_of_cities, vector<int>(instance.number_of_cities));

  // Reading cost_matrix
  for (int i = 0; i < instance.number_of_cities; ++i) {
    for (int j = 0; j < instance.number_of_cities; ++j) {
      file >> instance.cost_matrix[i][j];
    }
  }

  // Reading time_matrix
  for (int i = 0; i < instance.number_of_cities; ++i) {
    for (int j = 0; j < instance.number_of_cities; ++j) {
      file >> instance.time_matrix[i][j];
    }
  }

  // Reading passengers
  int max_cost, origin, destiny, max_time;
  for (int i = 0; i < instance.number_of_passengers; i++) {
    Passenger passenger;
    file >> max_cost >> origin >> destiny >> max_time;   
    passenger.max_cost = max_cost;
    passenger.origin = origin;
    passenger.destiny = destiny;
    passenger.max_time = max_time;
    instance.passengers.push_back(passenger);
  }

  // Reading min_quota
  file >> instance.min_quota;

  // Reading bonus_and_time
  vector<pair<int, int>> temp_bonus_and_time;
  vector<int> index;
  int city, bonus, time;
  while (file >> city >> bonus >> time) {
    temp_bonus_and_time.push_back(make_pair(bonus, time));
    index.push_back(city);
  }
  for(int i=0; i<temp_bonus_and_time.size(); i++){
    for(int j=0; j<temp_bonus_and_time.size(); j++){
      if(index[j] == i){
        instance.bonus_and_time.push_back(temp_bonus_and_time[j]);
        break;
      }
    }
  }

  file.close();

  return instance;
}

vector<string> generate_instance_paths() {
    vector<string> instances;
    vector<string> groups = {"A1","A2","A3"};
    vector<string> types = {"symmetric","asymmetric"};
    vector<int> sizes = {5, 8, 10, 20, 50, 100, 200};
    int files_per_instance = 6;

    for (const auto& group : groups) {
        for (const auto& type : types) {
            for (int size : sizes) {
                //for (int i = 1; i <= files_per_instance; ++i) {
                for (int i = 1; i <= files_per_instance; ++i) {
                    ostringstream path;
                    path << "instances/" << group << "/" << type << "/" << size << "." << i << ".in";
                    instances.push_back(path.str());
                }
            }
        }
    }

    return instances;
}

// Output functions
void print_solution_full(Instance instance, Solution &solution) {
  cout << "Rota: ";
  for(int i = 0; i < solution.route.size(); i++){
    cout << solution.route[i] << " ";
  }
  cout<<endl;

  cout<<"custos da rota:"<<endl;
  for(int city = 0; city < solution.route.size(); city++){
    int next_city;
    if(city == solution.route.size() - 1){
      next_city = 0;
    }
    else{
      next_city = city+1;
    }
    cout << instance.cost_matrix[solution.route[city]][solution.route[next_city]]<< " ";
  }
  cout<<endl;

  cout<<"Tempos da rota:"<<endl;
  for(int city = 0; city < solution.route.size(); city++){
    int next_city;
    if(city == solution.route.size() - 1){
      next_city = 0;
    }
    else{
      next_city = city+1;
    }
    cout << instance.time_matrix[solution.route[city]][solution.route[next_city]]<< " ";
  }
  cout<<endl;

  cout << "Cities_colected: "<<endl;
  bool any_city_colected = false;
  for(int i =0; i<solution.cities_colected.size();i++){
    if(solution.cities_colected[i]==true){
      cout<<"City: "<< solution.route[i] << " bonus " << instance.bonus_and_time[solution.route[i]].first << " tempo " << instance.bonus_and_time[solution.route[i]].second << endl;
      any_city_colected = true;
    }
  }
  if(!any_city_colected){
    cout<<"Nenhuma cidade coletada"<<endl;
  }
  cout<<endl;
}

void print_solution(Solution solution) {
  cout << "Route: ";
  for(int i = 0; i < solution.route.size(); i++){
    cout << solution.route[i] << " ";
  }
  cout<<endl;
  cout << "Cost: " << solution.cost << endl;
  cout << "Bonus: " << solution.total_bonus << endl;
  cout << "Time: " << solution.time << "\n \n";
}

void print_solution(Instance instance, Solution &solution) {
  cout << "Route: ";
  for(int i = 0; i < solution.route.size(); i++){
    cout << solution.route[i] << " ";
  }
  cout<<endl;

  cout<<"Route:"<<endl;
  for(int city = 0; city < solution.route.size(); city++){
    int next_city;
    if(city == solution.route.size() - 1){
      next_city = 0;
    }
    else{
      next_city = city+1;
    }
    cout << instance.cost_matrix[solution.route[city]][solution.route[next_city]]<< " ";
  }
  cout<<endl;

  cout<<"Route times:"<<endl;
  for(int city = 0; city < solution.route.size(); city++){
    int next_city;
    if(city == solution.route.size() - 1){
      next_city = 0;
    }
    else{
      next_city = city+1;
    }
    cout << instance.time_matrix[solution.route[city]][solution.route[next_city]]<< " ";
  }
  cout<<endl;

  cout << "Cities_colected: "<<endl;
  bool any_city_colected = false;
  for(int i =0; i<solution.cities_colected.size();i++){
    if(solution.cities_colected[i]==true){
      cout<<"City: "<< solution.route[i] << " bonus " << instance.bonus_and_time[solution.route[i]].first << " time " << instance.bonus_and_time[solution.route[i]].second << endl;
      any_city_colected = true;
    }
  }
  if(!any_city_colected){
    cout<<"No city colected"<<endl;
  }
  cout<<endl;
  int passengers_on = 0;
  for (int i = 0; i < solution.passengers_riding.size(); i++) {
    if (solution.passengers_riding[i]) {
      passengers_on++;
      cout<< "passenger "<<i<<" with origin "<<instance.passengers[i].origin<<", destin: "<<instance.passengers[i].destiny<< " max cost: "<<instance.passengers[i].max_cost<<" max time: "<<instance.passengers[i].max_time<<endl;
    }
  }
  cout << "boarded passengers: " << passengers_on << endl;
  cout << "Cost: " << solution.cost << endl;
  cout << "Bonus: " << solution.total_bonus << endl;
  cout << "Time: " << solution.time << "\n \n";
}

void print_population(Instance instance, Population population) {
  for (int i = 0; i < population.population.size(); i++) {
    cout << "Solução " << i << ":" << endl;
    print_solution(instance, population.population[i]);
  }
  cout << "Quantidade de soluções: " << population.population.size() << endl;
}

void shorter_print_population(Instance instance, Population population){
  cout<<"RESUMIDO (solução: custo tempo bonus)\n";
  for(int i =0; i<population.population.size(); i++){
    cout<<population.population[i].cost<<" "<<population.population[i].time<<" "<<population.population[i].total_bonus<<" "<<endl;
  }
  cout<<"tamanho da população: "<<population.population.size()<<endl;
}

void print_instance(Instance& instance){
  cout << "Number of cities: " << instance.number_of_cities << endl;
  cout << "Number of passengers: " << instance.number_of_passengers << endl;
  cout << "Car capacity: " << instance.car_capacity <<"\n " <<endl;
}

void print_fronts(Population population) {
  for(int i=0; i<population.fronts.size(); i++){
    cout << "Fronte " << i << ":" << endl;
    for(int j=0; j<population.fronts[i].size(); j++){
      cout<<population.fronts[i][j]<<" ";
    }
    cout<<endl;
  }
}

// File and directory management
void create_all_directories(int num_tests, int num_instances, std::string folder_name, vector<string> instances_addresses) {
  for (const std::string& algo : {"NSGA-II", "MOEAD", "SPEA2", "IBEA","MPLS"}) { 
    std::string algo_dir = folder_name + "/" + algo;
    for (int instance = 0; instance < num_instances; ++instance) {
        std::string instance_dir = algo_dir + "/"+instances_addresses[instance];  
          for (int test = 0; test < num_tests; ++test) {
            std::string teste_dir = instance_dir + "/test_" + std::to_string(test);
            filesystem::create_directories(teste_dir);
           }
        }
    }
}

void create_test_archives(
  const Instance& instance, 
  string algorithm_name, 
  string folder_name, 
  string instance_address, 
  int current_test, 
  Generations& saved_generations, 
  unsigned int seed, 
  duration<double> tempoTotal){
  for(int generation = 0; generation < saved_generations.generations.size(); generation++){
    string endereço_do_arquivo = folder_name+ "/"+algorithm_name+"/" + instance_address +"/"+ "test_" + to_string(current_test) +"/paretogeneration_" + to_string(generation) + ".txt";
    string endereço_do_arquivo_completo = folder_name+ "/"+algorithm_name+"/" + instance_address +"/"+ "test_" + to_string(current_test) +"/paretogeneration_" + to_string(generation) + "_complete.txt";
    string endereço_dados = folder_name+ "/"+algorithm_name+"/" + instance_address +"/"+ "test_" + to_string(current_test) + "/data";
    //cout<<"file path: "<<endereço_do_arquivo<<endl;
    ofstream arquivo(endereço_do_arquivo);
    ofstream arquivo_completo(endereço_do_arquivo_completo);
    ofstream arquivo_dados(endereço_dados);
    for(int pareto_set = 0; pareto_set< saved_generations.generations[generation].pareto_set.size() ; pareto_set++){
      arquivo<<saved_generations.generations[generation].pareto_set[pareto_set].cost<<" "<< saved_generations.generations[generation].pareto_set[pareto_set].time<<" " << saved_generations.generations[generation].pareto_set[pareto_set].total_bonus;
      fill_complete_archive(instance, arquivo_completo, saved_generations, generation, pareto_set);
      // add route, cities, passengers, costs, times, etc.
      if(pareto_set<saved_generations.generations[generation].pareto_set.size()-1){
        arquivo<<endl;
        arquivo_completo<<endl;
      }
    }
    arquivo_dados<< "seed: "<<seed<<endl;
    arquivo_dados<< "total time in seconds: "<<tempoTotal.count()<<endl;
    for(int duration =0;duration < saved_generations.durations.size(); duration++){
      arquivo_dados<<"time from "<<duration*10000<<" valuations to "<<(duration+1)*10000<<" valuations:"<<endl<< saved_generations.durations[duration].count()<<endl;
    }
    arquivo.close();
    arquivo_completo.close();
    arquivo_dados.close();
  }
}

void fill_complete_archive(
  const Instance& instance, 
  ofstream &arquivo_completo, 
  Generations& saved_generations, 
  int generation, 
  int pareto_set){
    arquivo_completo<<"Rota: ";
    for(int i = 0; i < saved_generations.generations[generation].solutions[pareto_set].route.size(); i++){
      arquivo_completo<< saved_generations.generations[generation].solutions[pareto_set].route[i] << " ";
    }
    arquivo_completo<<endl;
    arquivo_completo<<"custos da rota: ";
    for(int city = 0; city < saved_generations.generations[generation].solutions[pareto_set].route.size(); city++){
    int next_city;
    if(city == saved_generations.generations[generation].solutions[pareto_set].route.size() - 1){
      next_city = 0;
    }
    else{
      next_city = city+1;
    }
    arquivo_completo << instance.cost_matrix[saved_generations.generations[generation].solutions[pareto_set].route[city]][saved_generations.generations[generation].solutions[pareto_set].route[next_city]]<< " ";
  }
  arquivo_completo<<endl;

  arquivo_completo<<"Tempos da rota: ";
  for(int city = 0; city < saved_generations.generations[generation].solutions[pareto_set].route.size(); city++){
    int next_city;
    if(city == saved_generations.generations[generation].solutions[pareto_set].route.size() - 1){
      next_city = 0;
    }
    else{
      next_city = city+1;
    }
    arquivo_completo<< instance.time_matrix[saved_generations.generations[generation].solutions[pareto_set].route[city]][saved_generations.generations[generation].solutions[pareto_set].route[next_city]]<< " ";
  }
  arquivo_completo<<endl;

  arquivo_completo<< "Cities_colected: "<<endl;
  bool any_city_colected = false;
  for(int i =0; i<saved_generations.generations[generation].solutions[pareto_set].cities_colected.size();i++){
    if(saved_generations.generations[generation].solutions[pareto_set].cities_colected[i]==true){
      arquivo_completo<<"City: "<< saved_generations.generations[generation].solutions[pareto_set].route[i] << " bonus " << instance.bonus_and_time[saved_generations.generations[generation].solutions[pareto_set].route[i]].first << " tempo " << instance.bonus_and_time[saved_generations.generations[generation].solutions[pareto_set].route[i]].second << endl;
      any_city_colected = true;
    }
  }
  if(!any_city_colected){
    arquivo_completo<<"Nenhuma cidade coletada"<<endl;
  }
  int passengers_on = 0;
  for (int i = 0; i < saved_generations.generations[generation].solutions[pareto_set].passengers_riding.size(); i++) {
    if (saved_generations.generations[generation].solutions[pareto_set].passengers_riding[i]) {
      passengers_on++;
      arquivo_completo<< "passageiro "<<i<<" com origem "<<instance.passengers[i].origin<<", destino: "<<instance.passengers[i].destiny<< " custo max: "<<instance.passengers[i].max_cost<<" tempo max: "<<instance.passengers[i].max_time<<endl;
    }
  }
  arquivo_completo << "Passageiros embarcados: " << passengers_on << endl;
  arquivo_completo<< "Custo: " << saved_generations.generations[generation].solutions[pareto_set].cost << endl;
  arquivo_completo<< "Bonus: " << saved_generations.generations[generation].solutions[pareto_set].total_bonus << endl;
  arquivo_completo<< "Tempo: " << saved_generations.generations[generation].solutions[pareto_set].time << "\n \n";

}
