#pragma once
#include <iostream>
#include <vector>
#include <set>
#include "../include/Graph.h"

EdgeList calculateBFSTree (int noOfVertices, const std::vector<std::set<int>>& adjGraph);
std::vector<EdgeList> preprocessPredictedEdges(const Graph& graph);