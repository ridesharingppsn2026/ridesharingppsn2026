#pragma once

#include "main.h"
#include <vector>
#include <limits>
#include <numeric> 
#include <cmath>
using namespace std;

struct NDNode {
    vector<NDNode*> children;
    vector<Solution> solutions;
    bool is_leaf = true;
    NDNode* parent = nullptr;

    Objectives ideal_point;
    Objectives nadir_point;

    NDNode();
    ~NDNode();


    void updateIdealNadir(const Solution& y);
    void split(int max_children, int max_solutions);
    void insert(const Solution& y, int max_children, int max_solutions);
    void updateInternalNodesSolutions();
    bool updateNode(const Solution& y, bool& should_delete, int& current_size);
    NDNode* findClosestChild(const Solution& y);
    double calculateDistanceToMiddlePointOfChild(const Solution& y, const NDNode& child);
    Solution findPointWithMaxAvgDistance();
    Solution findPointFarthestFromChildren();
    void createChildWithPoint(const Solution& z, int max_solutions);
    void assignPointToClosestChild(const Solution& z);
    void printSolutions() const;
    void GetAllSolutions(vector<Solution>& allSolutions);
};

struct NDTree {
    NDNode* root = nullptr;
    int max_children;
    int max_solutions;
    int max_size;
    int current_size = 0;

    NDTree(int max_c, int max_s, int max_siz);
    ~NDTree();
    void update(const Solution& y);
    void printTree();
};
