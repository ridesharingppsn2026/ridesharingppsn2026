#ifndef WALKS_COMMON_H
#define WALKS_COMMON_H

#include "../landscapeElement.h"
#include "../landscapeMetrics.h"
#include "../../../utils/solution_utils.h"

extern int iLandscape;
using namespace std;
std::pair<double, int> calculate_decomp_metrics(LandscapeElement* element, std::vector<Solution>* neighborhood);
void calculate_dominance_metrics(LandscapeElement* element, vector<Solution>* neighborhood);
int number_pareto_front(LandscapeElement *landscape, vector<Solution>* neighborhood);
bool add_front_0(Solution &solution, vector<Solution> &front_0);

#endif