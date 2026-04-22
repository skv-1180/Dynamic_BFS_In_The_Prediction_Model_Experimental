#pragma once

#include <vector>
#include "Types.h"
#include "BFSState.h"

void classicalInsertEdge(BFSState& state, int u, int v);
void classicalDeleteEdge(BFSState& state, int u, int v);

std::vector<BFSSnapshot> preprocessIncremental(
    int           numVertices,
    int           source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates   // all must be INSERT
);

std::vector<BFSSnapshot> preprocessDecremental(
    int           numVertices,
    int           source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates   // all must be DELETE
);

std::vector<BFSSnapshot> preprocessFullyDynamic(
    int           numVertices,
    int           source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates   // may be INSERT or DELETE
);
