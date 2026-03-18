#include <vector>
#include <random>
#include <iostream>
#include <limits> 
#include <algorithm>
#include <cctype>
#include <fstream>
#include <chrono>
#include <filesystem>
#include <iomanip>


#include "./headers/metafeatures/walks/random_walk_pareto.h"
#include "./headers/metafeatures/walks/adaptative_walk_pareto.h"

#include "./headers/metafeatures/normalization.h"
#include "./headers/metafeatures/metrics_extraction.h"
#include "./headers/metafeatures/features_extraction.h"
#include "./headers/metafeatures/pareto_based/mo_features_pareto.h"
#include "./headers/metafeatures/landscapeMetrics.h"
#include "./headers/metafeatures/landscapeElement.h"
#include "./utils/features_csv.h"

#include "./headers/globals.h"

// Include MOTSPPP main structures
#include "./src/main.h"
#include "./utils/io_utils.h"
#include "./utils/solution_utils.h"

using namespace std;

vector<string> column_names_pareto = {
  "INF_avg", "INF_sd", "INF_r1", "INF_kr", "INF_sk",
  "SUP_avg", "SUP_sd", "SUP_r1", "SUP_kr", "SUP_sk",
  "INC_avg", "INC_sd", "INC_r1", "INC_kr", "INC_sk",
  "IND_avg", "IND_sd", "IND_r1", "IND_kr", "IND_sk",
};

int countPareto = 0;
int *countReval = &countPareto;

int iLandscape;
bool limitReached = false;
int limit = 160000;
int partial_limit = 80000;
bool partial_results_saved = false;
string folder_name_adaptative_walk;
string folder_name_random_walk;
vector<LandscapeElement> *updated_single_walk;
vector<LandscapeElement> *partial_walk_backup;
int walk_length = 8;
float percent_neighbors = 1.0;

// MOTSPPP specific globals
string instance_name;
string original_instance_path;
Instance* current_instance = nullptr;

// ||Log variables (definitions)
int current_max_neighbors = 0;
double current_percent_neighbors = 0.0;
double current_total_neighbors_analyzed = 0.0;
int current_number_of_subproblems = 0;
double last_run_seconds = 0.0;

vector<double> dominance_extraction(vector<LandscapeElement> &landscape){
  LandscapeMetrics metric = metric_extraction(landscape);

  return mo_features_extraction_pareto(metric);
}

void AP_first_improvement_main(int qtd_of_landscapes, int number_of_neighbors, string suffix = "") {
    auto single_adaptive_walk_P = adaptative_walk_pareto(number_of_neighbors, first_improvement_next_solution);
    auto AWP_mo_pareto_features = dominance_extraction(single_adaptive_walk_P);
    
    // Always generate CSV, regardless of limitReached
    string base_file = "ofe_" + to_string(*countReval) + suffix + ".csv";
    // Remove "instances/" prefix and ".in" suffix from path
    string path = original_instance_path;
    if(path.substr(0, 10) == "instances/") {
        path = path.substr(10);
    }
    if(path.size() > 3 && path.substr(path.size()-3) == ".in") {
        path = path.substr(0, path.size()-3);
    }
    // Build path using: dataset/<instance_path>/pareto_based/adaptative_first_improvement/r0.05/file
    build_csv(AWP_mo_pareto_features, column_names_pareto, "dataset", 
             path,                    // A1/asymmetric/200.1
             "pareto_based",         // moved here
             "adaptative_first_improvement",      // type of walk
             folder_name_adaptative_walk, // r0.05, etc
             base_file);
}

void AP_non_dominated_main(int qtd_of_landscapes, int number_of_neighbors, string suffix = "") {
    auto single_adaptive_walk_P = adaptative_walk_pareto(number_of_neighbors, non_dominated_set_next_solution);
    auto AWP_mo_pareto_features = dominance_extraction(single_adaptive_walk_P);
    
    // Always generate CSV, regardless of limitReached
    string base_file = "ofe_" + to_string(*countReval) + suffix + ".csv";
    // Remove "instances/" prefix and ".in" suffix from path
    string path = original_instance_path;
    if(path.substr(0, 10) == "instances/") {
        path = path.substr(10);
    }
    if(path.size() > 3 && path.substr(path.size()-3) == ".in") {
        path = path.substr(0, path.size()-3);
    }
    // Build path using: dataset/<instance_path>/pareto_based/adaptative_non_dominated/r0.05/file
    build_csv(AWP_mo_pareto_features, column_names_pareto, "dataset", 
             path,                    // A1/asymmetric/200.1
             "pareto_based",         // moved here
             "adaptative_non_dominated",      // type of walk
             folder_name_adaptative_walk, // r0.05, etc
             base_file);
}

void RW_pareto_main(int qtd_of_landscapes, int walk_lenght, int number_of_neighbors, string suffix = "") {
    auto single_random_walk = random_walk(walk_lenght, number_of_neighbors);
    auto RW_mo_pareto_features = dominance_extraction(single_random_walk);

    // Always generate CSV, regardless of limitReached
    string base_file = "ofe_" + to_string(*countReval) + suffix + ".csv";
    
    // Remove "instances/" prefix and ".in" suffix from path
    string path = original_instance_path;
    if(path.substr(0, 10) == "instances/") {
        path = path.substr(10);
    }
    if(path.size() > 3 && path.substr(path.size()-3) == ".in") {
        path = path.substr(0, path.size()-3);
    }
    
    // Build path using: dataset/<instance_path>/pareto_based/random_walk/l5_r0.05/file
    build_csv(RW_mo_pareto_features, column_names_pareto, "dataset",
             path,                // A1/asymmetric/200.1
             "pareto_based",     // moved here
             "random_walk",      // type of walk
             folder_name_random_walk, // l5_r0.05, etc
             base_file);
}

void generate_log_file(
    const string& log_base_dir,
    const string& base,
    const string& walk_type,
    const string& instance_name,
    const string& original_instance_path,
    const Instance* loaded_instance,
    int max_neighbors,
    double percent_neighbors,
    double total_neighbors_analyzed,
    int num_subproblems,
    double run_seconds,
    int total_evaluations) 
{
    // Create the path and file name for the log file
    string log_dir = log_base_dir; // Ex: "dataset/pareto_based_logs"
    std::filesystem::create_directories(log_dir);
    string log_file_path = log_dir + instance_name + "/" + base+"/" + walk_type +"/"+ original_instance_path+"/"+ "log.txt";
    string log_folder = log_dir + instance_name + "/" + base+"/" + walk_type +"/"+ original_instance_path;
    std::filesystem::create_directories(log_folder);
    // Open the file for writing
    ofstream log(log_file_path, ios::out);

    if (log.is_open()) {
        log << "============================== LOG ==============================\n";
          log << std::fixed << std::setprecision(12);
          log << "Maximum neighbors: " << current_max_neighbors << "\n";
          log << "Percentage of neighbors: " << current_percent_neighbors << "\n";
          log << "Total number of neighbors analyzed: " << current_total_neighbors_analyzed << "\n\n";
          log << "Number of subproblems: " << current_number_of_subproblems << "\n\n";
          log << "Number of cities: " << loaded_instance->number_of_cities << endl;
          log << "Number of passengers: " << loaded_instance->number_of_passengers << endl;
          log << "Car capacity: " << loaded_instance->car_capacity << endl << endl;
          log << "Run time (seconds): " << last_run_seconds << "\n\n";
          log << "TOTAL NUMBER OF AVALIATIONS: " << *countReval << "\n";
          log << "=================================================================\n";
          log.close();
    } else {
        cerr << "Error: Could not open log file for writing: " << log_file_path << endl;
    }
}

int main(int argc, char* argv[]){
try{
  int num_neighbors = 200;
  int size_of_population = 20;
  std::vector<double> r = {0.05, 0.1, 0.25, 0.5, 1.0};
  std::vector<string> rstring = {"0.05", "0.1", "0.25", "0.5", "1.0"};
  std::vector<int> l = {5, 10, 25, 50, 100};
  
  //||INSTANCE SETUP
  vector<string> instances_addresses = generate_instance_paths();
  int numero_de_instancias = instances_addresses.size();
  for(auto instance : instances_addresses){

    // ||Reset counters per instance
    *countReval = 0;
    limitReached = false;
    original_instance_path = instance; // ||store original path for CSV

    // Load MOTSPPP instance
    string full_path = "../" + instance;
    Instance loaded_instance = get_instance(full_path);
    current_instance = &loaded_instance;

    // ||Sanitize instance name for directory usage (replace '/')
    // ||Desired dataset format: GROUP_TYPE_BASE  ex: A1_ASYMMETRIC_5.1
   {
        std::string temp = instance; // instances/A1/asymmetric/200.1.in
        // Extract file name
        size_t last_slash = temp.find_last_of('/');
        std::string filename = (last_slash==std::string::npos)? temp : temp.substr(last_slash+1); // 200.1.in
        // Remove .in extension
        if(filename.size() > 3 && filename.substr(filename.size()-3)==".in"){
            filename = filename.substr(0, filename.size()-3); // 200.1
        }
        // Extract type (asymmetric) and group (A1)
        std::string path_without_file = (last_slash==std::string::npos)? std::string() : temp.substr(0,last_slash); // instances/A1/asymmetric
        size_t slash2 = path_without_file.find_last_of('/');
        std::string type = (slash2==std::string::npos)? path_without_file : path_without_file.substr(slash2+1); // asymmetric
        std::string path_without_type = (slash2==std::string::npos)? std::string() : path_without_file.substr(0,slash2); // instances/A1
        size_t slash3 = path_without_type.find_last_of('/');
        std::string group = (slash3==std::string::npos)? path_without_type : path_without_type.substr(slash3+1); // A1
        // ||Build final name
        instance_name = group + "/" + type + "/" + filename; // ||used only for folder structure
        cout<< "Instance nameeee: " << instance_name << endl;
      }


    cout << "Instance: " << instance << endl << endl;
    cout << "Maximum neighbors: " << num_neighbors << endl;
/*
    // ADAPTIVE FIRST IMPROVEMENT
    cout << "=== ADAPTIVE FIRST IMPROVEMENT ===" << endl;
    for(int i = 0; i < r.size(); i++){
      *countReval = 0;
      partial_results_saved = false;
      partial_walk_backup = nullptr;
      folder_name_adaptative_walk= "r"+ rstring[i]; // r0.05, r0.1, etc.
      percent_neighbors = r[i];
      current_percent_neighbors = percent_neighbors;
      current_max_neighbors = num_neighbors;
      current_total_neighbors_analyzed = num_neighbors * percent_neighbors;
      current_number_of_subproblems = size_of_population;
      
      cout << "Running First Improvement with r=" << rstring[i] << endl;
      auto start = std::chrono::high_resolution_clock::now();
      AP_first_improvement_main(size_of_population, num_neighbors * percent_neighbors, "_final");
      
      // Generate partial results if saved
      if(partial_results_saved && partial_walk_backup != nullptr) {
        cout << "Generating partial results at " << partial_limit << " evaluations..." << endl;
        auto partial_features = dominance_extraction(*partial_walk_backup);
        string base_file_partial = "ofe_" + to_string(partial_limit) + "_partial.csv";
        string path = original_instance_path;
        if(path.substr(0, 10) == "instances/") {
            path = path.substr(10);
        }
        if(path.size() > 3 && path.substr(path.size()-3) == ".in") {
            path = path.substr(0, path.size()-3);
        }
        build_csv(partial_features, column_names_pareto, "dataset", 
                 path, "pareto_based", "adaptative_first_improvement", 
                 folder_name_adaptative_walk, base_file_partial);
        delete partial_walk_backup;
        partial_walk_backup = nullptr;
      }
      
      auto end = std::chrono::high_resolution_clock::now();
      last_run_seconds = std::chrono::duration<double>(end - start).count();
      cout << "Final results at " << *countReval << " evaluations generated." << endl;
      generate_log_file(
          "dataset/",
          "pareto_based",
          "adaptative_first_improvement",
          instance_name,
          folder_name_adaptative_walk,
          current_instance,
          current_max_neighbors,
          current_percent_neighbors,
          current_total_neighbors_analyzed,
          current_number_of_subproblems,
          last_run_seconds,
          *countReval
      );
    }
    
    // ADAPTIVE NON-DOMINATED SET
    cout << "=== ADAPTIVE NON-DOMINATED SET ===" << endl;
    for(int i = 0; i < r.size(); i++){
      *countReval = 0;
      partial_results_saved = false;
      partial_walk_backup = nullptr;
      folder_name_adaptative_walk= "r"+ rstring[i]; // r0.05, r0.1, etc.
      percent_neighbors = r[i];
      current_percent_neighbors = percent_neighbors;
      current_max_neighbors = num_neighbors;
      current_total_neighbors_analyzed = num_neighbors * percent_neighbors;
      current_number_of_subproblems = size_of_population;
      
      cout << "Running Non-Dominated Set with r=" << rstring[i] << endl;
      auto start = std::chrono::high_resolution_clock::now();
      AP_non_dominated_main(size_of_population, num_neighbors * percent_neighbors, "_final");
      
      // Generate partial results if saved
      if(partial_results_saved && partial_walk_backup != nullptr) {
        cout << "Generating partial results at " << partial_limit << " evaluations..." << endl;
        auto partial_features = dominance_extraction(*partial_walk_backup);
        string base_file_partial = "ofe_" + to_string(partial_limit) + "_partial.csv";
        string path = original_instance_path;
        if(path.substr(0, 10) == "instances/") {
            path = path.substr(10);
        }
        if(path.size() > 3 && path.substr(path.size()-3) == ".in") {
            path = path.substr(0, path.size()-3);
        }
        build_csv(partial_features, column_names_pareto, "dataset", 
                 path, "pareto_based", "adaptative_non_dominated", 
                 folder_name_adaptative_walk, base_file_partial);
        delete partial_walk_backup;
        partial_walk_backup = nullptr;
      }
      
      auto end = std::chrono::high_resolution_clock::now();
      last_run_seconds = std::chrono::duration<double>(end - start).count();
      cout << "Final results at " << *countReval << " evaluations generated." << endl;
      generate_log_file(
          "dataset/",
          "pareto_based",
          "adaptative_non_dominated",
          instance_name,
          folder_name_adaptative_walk,
          current_instance,
          current_max_neighbors,
          current_percent_neighbors,
          current_total_neighbors_analyzed,
          current_number_of_subproblems,
          last_run_seconds,
          *countReval
      );
    }
*/      
    // RANDOM WALK
    cout << "=== RANDOM WALK ===" << endl;
    for(int i = 0; i < r.size(); i++){
      for(auto length: l) {
        *countReval = 0;
        partial_results_saved = false;
        partial_walk_backup = nullptr;
        folder_name_random_walk= "l"+to_string(length)+"_"+"r"+ rstring[i];
        walk_length = length;
        percent_neighbors = r[i];
        current_percent_neighbors = percent_neighbors;
        
        cout << "Running Random Walk with l=" << length << ", r=" << rstring[i] << endl;
        auto start = std::chrono::high_resolution_clock::now();
        RW_pareto_main(size_of_population, walk_length, num_neighbors * percent_neighbors, "_final");
        
        // Generate partial results if saved
        if(partial_results_saved && partial_walk_backup != nullptr) {
          cout << "Generating partial results at " << partial_limit << " evaluations..." << endl;
          auto partial_features = dominance_extraction(*partial_walk_backup);
          string base_file_partial = "ofe_" + to_string(partial_limit) + "_partial.csv";
          string path = original_instance_path;
          if(path.substr(0, 10) == "instances/") {
              path = path.substr(10);
          }
          if(path.size() > 3 && path.substr(path.size()-3) == ".in") {
              path = path.substr(0, path.size()-3);
          }
          build_csv(partial_features, column_names_pareto, "dataset", 
                   path, "pareto_based", "random_walk", 
                   folder_name_random_walk, base_file_partial);
          delete partial_walk_backup;
          partial_walk_backup = nullptr;
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        last_run_seconds = std::chrono::duration<double>(end - start).count();
        cout << "Final results at " << *countReval << " evaluations generated." << endl;
        // ||Generate log for each length
        generate_log_file(
          "dataset/",
          "pareto_based",
          "random_walk",
          instance_name,
          folder_name_random_walk,
          current_instance,
          current_max_neighbors,
          current_percent_neighbors,
          current_total_neighbors_analyzed,
          current_number_of_subproblems,
          last_run_seconds,
          *countReval
        );
      }

    }
    cout << endl;
    cout << "TOTAL NUMBER OF EVALUATIONS: " << *countReval << endl;
  }
} catch (const std::exception& e) {
    cerr << "An error occurred: " << e.what() << endl;
    return EXIT_FAILURE;
  }
  catch(...) {
          cout << "ERRO desconhecido na main!" << endl;
          return 1;
      }

 return 0;
}