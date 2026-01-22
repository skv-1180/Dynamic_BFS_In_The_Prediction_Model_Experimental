#pragma once
#include "../include/Graph.h"

EdgeList getPredictedEdges(
    int noOfVertices, 
    int noOfEdges, 
    int noOfAddionalEdges, 
    const EdgeList& initialEdges
);