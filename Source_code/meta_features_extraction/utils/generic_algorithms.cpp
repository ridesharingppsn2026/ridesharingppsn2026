#include "generic_algorithms.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <limits>
#include <numeric>

using namespace std;

// Swap functions
void swap(pair<double, int> &a, pair<double, int> &b) {
  pair<double, int> temp = a;
  a = b;
  b = temp;
}

void swap(int &a, int &b) {
  int temp = a;
  a = b;
  b = temp;
}

void swap(double &a, double &b) {
  double temp = a;
  a = b;
  b = temp;
}

// Heapify functions
void heapify(vector<pair<double, int>> &vec, int n, int i) {
  int largest = i;       // Initialize the largest as root
  int left = 2 * i + 1;  // left = 2*i + 1
  int right = 2 * i + 2; // right = 2*i + 2

  // If the left child is greater than the root
  if (left < n && vec[left].first > vec[largest].first) {
    largest = left;
  }

  // If the right child is greater than the root
  if (right < n && vec[right].first > vec[largest].first) {
    largest = right;
  }

  // If the biggest is not the root
  if (largest != i) {
    swap(vec[i], vec[largest]);
    // Recursively heapify the affected subtree
    heapify(vec, n, largest);
  }
}

void heapify(vector<int> &vec, int n, int i) {
  int largest = i;       // Initialize the largest as root
  int left = 2 * i + 1;  // left = 2*i + 1
  int right = 2 * i + 2; // right = 2*i + 2

  // If the left child is greater than the root
  if (left < n && vec[left] > vec[largest]) {
    largest = left;
  }

  // If the right child is greater than the root
  if (right < n && vec[right] > vec[largest]) {
    largest = right;
  }

  // If the biggest is not the root
  if (largest != i) {
    swap(vec[i], vec[largest]);
    // Recursively heapify the affected subtree
    heapify(vec, n, largest);
  }
}

void heapify(vector<double> &vec, int n, int i) {
  int largest = i;       // Initialize the largest as root
  int left = 2 * i + 1;  // left = 2*i + 1
  int right = 2 * i + 2; // right = 2*i + 2

  // If the left child is greater than the root
  if (left < n && vec[left] > vec[largest]) {
    largest = left;
  }

  // If the right child is greater than the root
  if (right < n && vec[right] > vec[largest]) {
    largest = right;
  }

  // If the biggest is not the root
  if (largest != i) {
    swap(vec[i], vec[largest]);
    // Recursively heapify the affected subtree
    heapify(vec, n, largest);
  }
}

// HeapSort functions
void heapSort(vector<pair<double, int>> &vec) {
  int n = vec.size();

  // Builds the heap (rearranges the vector)
  for (int i = n / 2 - 1; i >= 0; i--) {
    heapify(vec, n, i);
  }

  // Extracts one element from the heap at a time
  for (int i = n - 1; i >= 0; i--) {
    // Move current root to the end
    swap(vec[0], vec[i]);
    // Call heapify on the reduced heap
    heapify(vec, i, 0);
  }
}

void heapSort(vector<int> &vec) {
  int n = vec.size();

  // Builds the heap (rearranges the vector)
  for (int i = n / 2 - 1; i >= 0; i--) {
    heapify(vec, n, i);
  }

  // Extracts one element from the heap at a time
  for (int i = n - 1; i >= 0; i--) {
    // Move current root to the end
    swap(vec[0], vec[i]);
    // Call heapify on the reduced heap
    heapify(vec, i, 0);
  }
}

void heapSort(vector<double> &vec) {
  int n = vec.size();

  // Builds the heap (rearranges the vector)
  for (int i = n / 2 - 1; i >= 0; i--) {
    heapify(vec, n, i);
  }

  // Extracts one element from the heap at a time
  for (int i = n - 1; i >= 0; i--) {
    // Move current root to the end
    swap(vec[0], vec[i]);
    // Call heapify on the reduced heap
    heapify(vec, i, 0);
  }
}
