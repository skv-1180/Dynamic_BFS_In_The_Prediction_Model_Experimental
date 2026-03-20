#pragma once

// ============================================================
// BFSState.h
// A "working copy" of the full BFS state: adjacency lists
// (in and out), level array, parent array, and upper-parent
// sets.  Used both as the real-graph tracker and as the
// temporary state for the batch algorithms.
// ============================================================

#include <vector>
#include <set>
#include <iostream>
#include "Config.h"
#include "Types.h"

class BFSState {
public:
    int n{};       // |V|, vertices are 1-indexed from 1..n
    int source{};  // fixed source vertex

    std::vector<int>            level;   // level[v]
    std::vector<int>            parent;  // parent[v]
    std::vector<std::set<int>>  UP;      // upper-parents UP[v]
    std::vector<std::set<int>>  outAdj;  // out-neighbours Out[v]
    std::vector<std::set<int>>  inAdj;   // in-neighbours  In[v]

    // Construct a blank state for n vertices and the given source.
    BFSState(int n, int source);

    // Copy constructor / assignment (deep copy of all sets).
    BFSState(const BFSState&)            = default;
    BFSState& operator=(const BFSState&) = default;

    // -------------------------------------------------------
    // Adjacency mutation helpers
    // -------------------------------------------------------
    void addEdge   (int u, int v);   // insert directed edge u->v
    void removeEdge(int u, int v);   // remove directed edge u->v

    // -------------------------------------------------------
    // BFS tree computation
    // -------------------------------------------------------
    // Recompute level[] and parent[] from scratch via BFS.
    void computeBFS();

    // Compute ALL upper-parent sets UP[v] from the current
    // level[] and adjacency.  Call AFTER computeBFS().
    void computeAllUP();

    // -------------------------------------------------------
    // Snapshot I/O
    // -------------------------------------------------------
    // Write current level/parent/UP into a lightweight snapshot.
    BFSSnapshot saveSnapshot(bool saveUP = false) const;

    // Restore level/parent/(optionally UP) from a snapshot.
    void loadSnapshot(const BFSSnapshot& snap);

    // -------------------------------------------------------
    // Diagnostics
    // -------------------------------------------------------
    void printBFSTree(std::ostream& os = std::cout) const;
    void printAdjacency(std::ostream& os = std::cout) const;

    // Verify that level[] is consistent with the current
    // adjacency (useful for unit-testing).
    bool verify() const;
};
