#pragma once
#include <vector>

#include "BFSState.h"
#include "Types.h"

// Offline preprocessing
// snapshots[0]  = BFS of G_0 (initial graph),
// snapshots[i]  = BFS of Ĝ_i (after i-th predicted insertion).

void classicalInsertEdge(BFSState& state, int u, int v);
void classicalDeleteEdge(BFSState& state, int u, int v);

std::vector<BFSSnapshot> preprocessIncremental(
    int numVertices,
    int source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates  // all must be INSERT
);

std::vector<BFSSnapshot> preprocessDecremental(
    int numVertices,
    int source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates  // all must be DELETE
);
 
std::vector<BFSSnapshot> preprocessFullyDynamic(
    int numVertices,
    int source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates);
