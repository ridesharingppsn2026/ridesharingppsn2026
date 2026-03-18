#ifndef GLOBALS_H
#define GLOBALS_H

#include <vector>
#include <string>
#include <utility>
#include "./metafeatures/landscapeElement.h"

extern int iLandscape;
extern int* countReval;
extern int defaultDomPace;
extern std::vector<LandscapeElement> *updated_single_walk;
extern std::vector<std::string> column_names_pareto;
extern std::string folder_name_adaptative_walk;
extern std::string folder_name_random_walk;
extern int walk_length;
extern float percent_neighbors;
extern std::string instance_path;
extern std::vector<std::pair<double, double>> lambda_vector;
extern std::pair<double, double> global_z_point;
extern double maximal;
extern double minimal;
extern bool limitReached;
extern int limit;
extern int partial_limit;
extern bool partial_results_saved;
extern std::vector<LandscapeElement> *partial_walk_backup;
 
// MOTSPPP specific globals
extern std::string instance_name;
extern std::string original_instance_path; // ||new: full original path to save in CSV
struct Instance;  
extern Instance* current_instance;

// Logging context
extern int current_max_neighbors;
extern double current_percent_neighbors;
extern double current_total_neighbors_analyzed;
extern int current_number_of_subproblems;
extern double last_run_seconds;

#endif
