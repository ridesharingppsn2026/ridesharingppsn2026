#include "isEqual.h"

bool isEqual(Solution solutionA, Solution solutionB) {
  return (solutionA.cost == solutionB.cost) && (solutionA.time == solutionB.time) && (solutionA.total_bonus == solutionB.total_bonus);
}