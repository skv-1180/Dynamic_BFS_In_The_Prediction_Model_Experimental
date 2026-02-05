#pragma once

#include <vector>
#include <string>
#include <unordered_map>
#include "../include/Graph.h"

// Entry recording that vertex v's parent becomes 'parent' with distance 'dist' at some update.
struct BFSEntry {
    int v;
    int parent;
    int dist;
};

using BFSEntryList = std::vector<BFSEntry>;

class BFSSnapshotStore {
private:
    // per-update change lists: changeLists_[t] contains BFSEntry for update t (0-based)
    std::vector<BFSEntryList> changeLists_;

    // map from predicted-edge key ("u_v_type") to update index (0-based)
    std::unordered_map<std::string, int> edgeToIndex_;

    // initial parent/dist arrays (for quick reconstruct)
    std::vector<int> initialParent_;
    std::vector<int> initialDist_;

    int nVertices_ = 0;

    static std::string makeEdgeKey(const Edge& e);

public:
    BFSSnapshotStore() = default;

    // Build change-lists from the graph (uses initial edges + graph.getPredictedEdges()).
    void buildFromGraph(const Graph& graph);

    // Number of predicted updates (change-lists) stored.
    int snapshotCount() const;

    // Access the per-update change-list (0-based index).
    const BFSEntryList& getChangeList(int idx) const;

    // Look up update index by predicted edge (returns -1 if not found)
    int indexOfPredictedEdge(const Edge& e) const;

    // Reconstruct and return the full BFS tree (EdgeList of parent->child) at update index idx (0-based).
    // If idx < 0 or idx >= snapshotCount() throw std::out_of_range.
    EdgeList getBFSTreeAtIndex(int idx) const;

    // Reconstruct all full BFS trees (useful for printing / compatibility with Graph::setPreprocessedBFSTreeEdges)
    std::vector<EdgeList> reconstructAllFullTrees() const;
};
