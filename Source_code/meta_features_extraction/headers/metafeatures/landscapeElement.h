#ifndef LANDSCAPE_ELEMENT_H
#define LANDSCAPE_ELEMENT_H

#include <vector>

#include "../../src/main.h"
// Forward declarations to avoid circular dependencies

struct LandscapeElement {
  Solution current_solution;
  double tch_current_solution;
  double inf; 
  double sup; 
  double inc; 
  double ind; 
  std::vector<double> tchebycheff_neighbors;
};

#endif 
