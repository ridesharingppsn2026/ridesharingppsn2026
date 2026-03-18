#ifndef METRICS_EXTRACTION_H
#define METRICS_EXTRACTION_H

#include <vector>
#include "./landscapeMetrics.h"
#include "./landscapeElement.h"

LandscapeMetrics metric_extraction(std::vector<LandscapeElement> &landscape);

std::vector<LandscapeMetrics> metrics_extraction(std::vector<std::vector<LandscapeElement>> &landscapes);

#endif 
