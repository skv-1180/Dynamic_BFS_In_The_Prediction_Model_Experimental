#pragma once
#include <vector>
#include <queue>

void calculateBFSTree (
    int noOfVertices, 
    const std::vector<std::vector<int>>& adjGraph,
    std::vector<int>& dist,
    std::vector<int>& parent
);