#include <vector>
#include <utility>
#include <iostream>
#include <memory>

#include "../../headers/metafeatures/landscapeElement.h"
#include "../../headers/metafeatures/walks/adaptative_walk_decomposition.h"
#include "../../utils/solution_utils.h"
#include "../../headers/metafeatures/features_extraction.h"
#include "../../headers/metafeatures/walks/walks_common.h"

#include "../../headers/globals.h"

using namespace std;

// ||MOTSPPP: 3 objectives - minimize cost, minimize time, maximize bonus


vector<LandscapeElement> adaptative_walk_pareto(int number_of_neighbors, Solution (*next_solution) (const LandscapeElement &, const std::vector<Solution>*)){

  vector<LandscapeElement> S;
  updated_single_walk = &S;
  
  // ||Use MOTSPPP function to create initial solution
  Solution current_solution = get_random_solution(*current_instance);
  *countReval = *countReval + 1;

  if (*countReval == limit){
    limitReached = true;
    return S;
  }

  while (true) {
    LandscapeElement element;
    element.current_solution = current_solution;
    auto neighborhood = std::make_unique<std::vector<Solution>>();
    Solution neighbor;

    
    for(int i = 0; i < number_of_neighbors; i++){
      float stop_looking_percent = 0.1; // 10% of the limit
      neighbor = get_neighbor(current_solution);
      int executioncount =0;
      if(*countReval == limit){
        limitReached = true;
        return S;
      }

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
      // neighborhood->push_back(neighbor);
    }

    calculate_dominance_metrics(&element, neighborhood.get());
    S.push_back(element);

    // Check for partial results at 80k evaluations
    if(*countReval >= partial_limit && !partial_results_saved) {
      partial_walk_backup = new vector<LandscapeElement>(S);
      partial_results_saved = true;
    }

    current_solution = next_solution(element, neighborhood.get());

    // ||Comparison using the 3 MOTSPPP objectives: 
    // ||minimize cost, minimize time, maximize bonus
    if(current_solution.cost == element.current_solution.cost && 
       current_solution.time == element.current_solution.time && 
       current_solution.total_bonus == element.current_solution.total_bonus){
      break;
    }
  }
  return S;
}

// ||First Improvement: return the first solution that dominates the current one
Solution first_improvement_next_solution(const LandscapeElement &element, const vector<Solution>* neighborhood){
  Solution best_solution = element.current_solution;

  for(int i = 0; i < (*neighborhood).size(); i++){
    if(dominates((*neighborhood)[i], best_solution)){
      return (*neighborhood)[i];  // Retorna imediatamente a primeira que domina
    }
  }

  return best_solution;
}

// ||Non-Dominated Set: randomly select among non-dominated solutions
// ||ORIGINAL IMPLEMENTATION (||ISSUE: compara vizinhos entre si + só aceita se dominar) - ||COMMENTED
/*
Solution non_dominated_set_next_solution(const LandscapeElement &element, const vector<Solution>* neighborhood){
  Solution best_solution = element.current_solution;
  vector<Solution> non_dominated_solutions;
  
  // Coleta todas as soluções não dominadas ENTRE SI (||ISSUE)
  for(int i = 0; i < (*neighborhood).size(); i++){
    bool is_dominated = false;
    for(int j = 0; j < (*neighborhood).size(); j++){
      if(i != j && dominates((*neighborhood)[j], (*neighborhood)[i])){
        is_dominated = true;
        break;
      }
    }
    if(!is_dominated){
      non_dominated_solutions.push_back((*neighborhood)[i]);
    }
  }
  
  // Se há soluções não dominadas, seleciona uma aleatoriamente
  if(!non_dominated_solutions.empty()){
    int random_index = rand() % non_dominated_solutions.size();
    Solution selected = non_dominated_solutions[random_index];
    
    // Se a selecionada domina a atual, retorna ela; senão, retorna a atual (||ISSUE)
    if(dominates(selected, best_solution)){
      return selected;
    }
  }

  return best_solution;
}
*/

// ||PREVIOUS IMPLEMENTATION (apenas compara com atual) - ||COMMENTED
/*
Solution non_dominated_set_next_solution(const LandscapeElement &element, const vector<Solution>* neighborhood){
  Solution best_solution = element.current_solution;
  vector<Solution> non_dominated_solutions;
  
  // Coleta soluções que NÃO são dominadas pela solução atual
  for(int i = 0; i < (*neighborhood).size(); i++){
    if(!dominates(best_solution, (*neighborhood)[i])){ // Se atual NÃO domina vizinho
      non_dominated_solutions.push_back((*neighborhood)[i]);
    }
  }
  
  // Seleciona aleatoriamente entre as não dominadas pela atual
  if(!non_dominated_solutions.empty()){
    int random_index = rand() % non_dominated_solutions.size();
    return non_dominated_solutions[random_index];
  }

  return best_solution;
}
*/

// ||NEW CORRECT IMPLEMENTATION (||Neighborhood Pareto Front + current)
Solution non_dominated_set_next_solution(const LandscapeElement &element, const vector<Solution>* neighborhood){
  Solution best_solution = element.current_solution;
  vector<Solution> non_dominated_solutions;
  
  // ||Create full set: neighborhood + current solution
  vector<Solution> all_solutions = *neighborhood;
  all_solutions.push_back(best_solution);
  
  // ||Collect neighborhood solutions that form the Pareto Front
  for(int i = 0; i < (*neighborhood).size(); i++){
    bool is_dominated = false;
    
    // Verifica se vizinho i é dominado por qualquer solução (incluindo atual)
    for(int j = 0; j < all_solutions.size(); j++){
      // Compara vizinho i com todas as outras soluções
      if(dominates(all_solutions[j], (*neighborhood)[i])){
        is_dominated = true;
        break;
      }
    }
    
    // Se não é dominado por nenhuma, faz parte do Pareto Front
    if(!is_dominated){
      non_dominated_solutions.push_back((*neighborhood)[i]);
    }
  }
  
  // Seleciona aleatoriamente do Pareto Front
  if(!non_dominated_solutions.empty()){
    int random_index = rand() % non_dominated_solutions.size();
    return non_dominated_solutions[random_index];
  }

  return best_solution;
}

// ||Compatibility with existing code (same as first_improvement)
Solution pareto_next_solution(const LandscapeElement &element, const vector<Solution>* neighborhood){
  return first_improvement_next_solution(element, neighborhood);
}
