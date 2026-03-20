#pragma once

// ============================================================
// Preprocess.h
// Offline preprocessing: simulate the predicted update sequence
// and store a BFS snapshot after every predicted step.
//
// Incremental  – O(mn) time and space (n vertices × m snapshots)
// Decremental  – O(m²) time and space (also stores UP sets)
// FullyDynamic – O(m²) time and space (trivial BFS after each step)
// ============================================================

#include <vector>
#include "Types.h"
#include "BFSState.h"

// Returns snapshots[0..m], where
//   snapshots[0]  = BFS of G_0 (initial graph),
//   snapshots[i]  = BFS of Ĝ_i (after i-th predicted insertion).
//
// Snapshots contain only level and parent (no UP).
std::vector<BFSSnapshot> preprocessIncremental(
    int           numVertices,
    int           source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates   // all must be INSERT
);

// Returns snapshots[0..m], where
//   snapshots[i] = BFS of Ĝ_i (after i-th predicted deletion).
//
// Snapshots contain level, parent AND upper-parent sets (needed
// for batchDeleteEdge).
std::vector<BFSSnapshot> preprocessDecremental(
    int           numVertices,
    int           source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates   // all must be DELETE
);

// Returns snapshots[0..m].
//   snapshots[i] = BFS of Ĝ_i via full BFS recomputation.
//
// Snapshots contain level, parent AND upper-parent sets.
std::vector<BFSSnapshot> preprocessFullyDynamic(
    int           numVertices,
    int           source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates   // may be INSERT or DELETE
);
