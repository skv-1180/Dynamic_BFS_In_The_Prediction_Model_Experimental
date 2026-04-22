#pragma once

#include <set>
#include <string>
#include <unordered_set>
#include <vector>
#include "Config.h"

struct EdgeUpdate
{
    int u{};            // tail of the directed edge  u -> v
    int v{};            // head
    UpdateType type{};  // INSERT or DELETE

    bool operator==(const EdgeUpdate& o) const
    {
        return u == o.u && v == o.v && type == o.type;
    }
    bool operator!=(const EdgeUpdate& o) const { return !(*this == o); }
};

struct EdgeUpdateHash
{
    size_t operator()(const EdgeUpdate& e) const
    {
        size_t h1 = std::hash<int>{}(e.u);
        size_t h2 = std::hash<int>{}(e.v);
        size_t h3 = std::hash<int>{}(static_cast<int>(e.type));

        return h1 ^ (h2 << 1) ^ (h3 << 2);
    }
};

using EdgeList = std::vector<EdgeUpdate>;
using SimpleEdge = std::pair<int, int>;  // (u, v) without type

struct BFSSnapshot
{
    std::vector<int> level;                             // level[v], 1-indexed; INF_LEVEL if unreachable
    std::vector<int> parent;                            // parent[v], NO_PARENT if root / unreachable
    std::vector<std::unordered_set<int>> upperParents;  // UP[v] = {u∈In[v] : level[u]=level[v]-1}
                                                        // populated only for dec/fully-dynamic
    bool hasUpperParents{false};                        // flag: do upperParents contain valid data?
};

struct QueryResult
{
    int step{};               // which real update was just processed
    std::vector<int> level;   // level[v] in BFS tree of G_j
    std::vector<int> parent;  // parent[v]
    bool usedPrediction{};    // true when snapshot was used directly (Case 1)
    int lastMatchedStep{};    // i at the time of this query
};
