#include <vector>
#include <utility>
#include <iostream>
#include <memory>

#include "../../headers/metafeatures/landscapeElement.h"
#include "../../headers/metafeatures/walks/random_walk_pareto.h"
#include "../../utils/solution_utils.h"
#include "../../utils/io_utils.h"
#include "../../headers/metafeatures/features_extraction.h"
#include "../../headers/metafeatures/walks/walks_common.h"

#include "../../headers/globals.h"

using namespace std;

// ||MOTSPPP: 3 objectives - minimize cost, minimize time, maximize bonus

vector<LandscapeElement> random_walk(int walk_lenght, int number_of_neighbors){

  vector<LandscapeElement> S;
  updated_single_walk = &S;
  
  // ||Use MOTSPPP function to create initial solution
  Solution current_solution = get_random_solution(*current_instance);
  *countReval = *countReval + 1;

  if (*countReval == limit){
    limitReached = true;
    return S;
  }

  for(int step = 0; step < walk_lenght; step++){
    LandscapeElement element;
    element.current_solution = current_solution; 
    auto neighborhood = std::make_unique<std::vector<Solution>>();
    Solution neighbor;

    
    for(int i = 0; i < number_of_neighbors; i++){
      float stop_looking_percent = 0.1; // 10% of the limit
      neighbor = get_neighbor(current_solution);

      if(*countReval == limit){
        limitReached = true;
        return S;
      }
      int executioncount = 0;
      while(isEqualNeighborhood(&neighbor, neighborhood.get()) and executioncount < round(limit*stop_looking_percent)){
        neighbor = get_neighbor(current_solution);
        executioncount++;
        if(*countReval == limit){
          limitReached = true;
          return S;
        }
      }
      if(executioncount < round(limit*stop_looking_percent)) {
        // Only add the neighbor if we found a unique one
        neighborhood->push_back(neighbor);
      }else{
        break;
      }
    }

    calculate_dominance_metrics(&element, neighborhood.get());
    S.push_back(element);

    // Check for partial results at 80k evaluations
    if(*countReval >= partial_limit && !partial_results_saved) {
      partial_walk_backup = new vector<LandscapeElement>(S);
      partial_results_saved = true;
    }

    Solution random_neighbor = (*neighborhood)[rand() % neighborhood->size()];
    current_solution = random_neighbor;
  }

  return S;
}