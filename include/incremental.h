#pragma once
#include <iostream>
#include <vector>
#include <cassert>
#include "../include/Graph.h"

BFSEntryList insertEdge(
    const Edge& newEdge,
    std::vector<std::vector<int>>& adjGraph,
    std::vector<int>& dist,
    std::vector<int>& parent
);

void preprocessPredictedEdges(Graph& graph);
