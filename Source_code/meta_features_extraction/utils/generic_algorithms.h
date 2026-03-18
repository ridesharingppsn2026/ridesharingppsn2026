#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>
#include <numeric>

// Swap functions
void swap(std::pair<double, int> &a, std::pair<double, int> &b);
void swap(int &a, int &b);
void swap(double &a, double &b);

// Heapify functions
void heapify(std::vector<std::pair<double, int>> &vec, int n, int i);
void heapify(std::vector<int> &vec, int n, int i);
void heapify(std::vector<double> &vec, int n, int i);

// HeapSort functions
void heapSort(std::vector<std::pair<double, int>> &vec);
void heapSort(std::vector<int> &vec);
void heapSort(std::vector<double> &vec);
