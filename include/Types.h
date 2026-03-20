#pragma once

// ============================================================
// Types.h
// Core data-types shared across all modules.
// ============================================================

#include <vector>
#include <set>
#include <string>
#include "Config.h"

// -----------------------------------------------------------
// A single directed-edge update (insertion or deletion).
// -----------------------------------------------------------
struct EdgeUpdate {
    int u{};            // tail of the directed edge  u -> v
    int v{};            // head
    UpdateType type{};  // INSERT or DELETE

    bool operator==(const EdgeUpdate& o) const {
        return u == o.u && v == o.v && type == o.type;
    }
    bool operator!=(const EdgeUpdate& o) const { return !(*this == o); }
};

using EdgeList    = std::vector<EdgeUpdate>;
using SimpleEdge  = std::pair<int,int>; // (u, v) without type

// -----------------------------------------------------------
// A lightweight snapshot of the BFS tree after predicted step i.
// Stores level, parent and (optionally) upper-parent sets.
// Does NOT store graph adjacency — that is reconstructed on
// demand by replaying the real update history up to the last
// matched step.
// -----------------------------------------------------------
struct BFSSnapshot {
    std::vector<int>         level;        // level[v], 1-indexed; INF_LEVEL if unreachable
    std::vector<int>         parent;       // parent[v], NO_PARENT if root / unreachable
    std::vector<std::set<int>> upperParents; // UP[v] = {u∈In[v] : level[u]=level[v]-1}
                                             // populated only for dec/fully-dynamic
    bool hasUpperParents{false};           // flag: do upperParents contain valid data?
};

// -----------------------------------------------------------
// The complete input to the algorithm.
// -----------------------------------------------------------
struct ProblemInstance {
    int      numVertices{};
    int      source{DEFAULT_SOURCE};
    EdgeList initialEdges;      // edges present in G_0 (all are insertions)
    EdgeList predictedUpdates;  // Û sequence, length m
    EdgeList realUpdates;       // U  sequence, length m
};

// -----------------------------------------------------------
// Result returned to the user after processing update j.
// -----------------------------------------------------------
struct QueryResult {
    int              step{};          // which real update was just processed
    std::vector<int> level;           // level[v] in BFS tree of G_j
    std::vector<int> parent;          // parent[v]
    bool             usedPrediction{};// true when snapshot was used directly (Case 1)
    int              lastMatchedStep{};// i at the time of this query
};
