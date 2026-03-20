#pragma once
#include <set>
#include <string>
#include <vector>
#include "Config.h"

struct EdgeUpdate {
    int u{};  // u -> v
    int v{};
    UpdateType type{};

    bool operator==(const EdgeUpdate& o) const {
        return u == o.u && v == o.v && type == o.type;
    }
    bool operator!=(const EdgeUpdate& o) const { return !(*this == o); }
};

using EdgeList = std::vector<EdgeUpdate>;
using SimpleEdge = std::pair<int, int>;  // without type

struct BFSSnapshot {
    std::vector<int> level;  // INF_LEVEL if unreachable
    std::vector<int> parent;
    std::vector<std::set<int>> upperParents;  // populated only for dec/fully-dynamic
    bool hasUpperParents{false};              // do upperParents contain valid data?
};

struct ProblemInstance {
    int numVertices{};
    int source{DEFAULT_SOURCE};
    EdgeList initialEdges;
    EdgeList predictedUpdates;
    EdgeList realUpdates;
};

struct QueryResult {
    int step{};  // which real update was just processed
    std::vector<int> level;
    std::vector<int> parent;
    bool usedPrediction{};  // true when stored bfs tree was used directly
    int lastMatchedStep{};
};
