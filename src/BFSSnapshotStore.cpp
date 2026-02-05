#include "../include/BFSSnapshotStore.h"
#include <set>
#include <queue>
#include <sstream>
#include <stdexcept>

static void computeParentAndDist(int n, const std::vector<std::set<int>>& adj,
                                 std::vector<int>& parent, std::vector<int>& dist) {
    parent.assign(n + 1, 0);
    dist.assign(n + 1, -1);
    int source = 1;
    std::queue<int> q;
    dist[source] = 0;
    parent[source] = 0;
    q.push(source);

    while (!q.empty()) {
        int u = q.front();
        q.pop();
        for (int v : adj[u]) {
            if (dist[v] != -1) continue;
            dist[v] = dist[u] + 1;
            parent[v] = u;
            q.push(v);
        }
    }
}

std::string BFSSnapshotStore::makeEdgeKey(const Edge& e) {
    std::ostringstream oss;
    oss << e.u << "_" << e.v << "_" << static_cast<int>(e.type);
    return oss.str();
}

void BFSSnapshotStore::buildFromGraph(const Graph& graph) {
    // Initialize
    nVertices_ = graph.getNumOfVertices();
    changeLists_.clear();
    edgeToIndex_.clear();
    initialParent_.clear();
    initialDist_.clear();

    // Build adjacency from initial edges (undirected as in your preprocess)
    std::vector<std::set<int>> adj(nVertices_ + 1);
    EdgeList initial = graph.getInitialEdges();
    for (const auto& e : initial) {
        adj[e.u].insert(e.v);
        adj[e.v].insert(e.u);
    }

    // compute initial BFS parent/dist
    computeParentAndDist(nVertices_, adj, initialParent_, initialDist_);

    // iterate predicted edges and compute change-lists
    EdgeList predicted = graph.getPredictedEdges();
    int mPred = static_cast<int>(predicted.size());

    std::vector<int> prevParent = initialParent_;
    std::vector<int> prevDist = initialDist_;

    for (int i = 0; i < mPred; ++i) {
        const Edge& pe = predicted[i];
        // apply predicted update to adjacency
        if (!pe.type) { // insertion
            adj[pe.u].insert(pe.v);
            adj[pe.v].insert(pe.u);
        } else { // deletion
            adj[pe.u].erase(pe.v);
            adj[pe.v].erase(pe.u);
        }

        // compute new parent/dist
        std::vector<int> newParent, newDist;
        computeParentAndDist(nVertices_, adj, newParent, newDist);

        // find differences (Only store vertices whose parent changes)
        BFSEntryList diffs;
        for (int v = 1; v <= nVertices_; ++v) {
            if (newParent[v] != prevParent[v]) {
                BFSEntry be;
                be.v = v;
                be.parent = newParent[v];
                be.dist = newDist[v];
                diffs.push_back(be);
            }
        }

        changeLists_.push_back(std::move(diffs));

        // map predicted edge key to index
        edgeToIndex_[makeEdgeKey(pe)] = i;

        // advance prev
        prevParent.swap(newParent);
        prevDist.swap(newDist);
    }
}

int BFSSnapshotStore::snapshotCount() const {
    return static_cast<int>(changeLists_.size());
}

const BFSEntryList& BFSSnapshotStore::getChangeList(int idx) const {
    if (idx < 0 || idx >= snapshotCount()) throw std::out_of_range("change list index out of range");
    return changeLists_[idx];
}

int BFSSnapshotStore::indexOfPredictedEdge(const Edge& e) const {
    auto it = edgeToIndex_.find(makeEdgeKey(e));
    if (it == edgeToIndex_.end()) return -1;
    return it->second;
}

EdgeList BFSSnapshotStore::getBFSTreeAtIndex(int idx) const {
    if (idx < 0 || idx >= snapshotCount()) throw std::out_of_range("snapshot index out of range");

    // reconstruct by applying updates up to idx
    std::vector<int> parent = initialParent_;
    // apply diffs sequentially
    for (int t = 0; t <= idx; ++t) {
        const BFSEntryList& diffs = changeLists_[t];
        for (const auto& be : diffs) {
            parent[be.v] = be.parent;
        }
    }

    // convert parent array to EdgeList (parent -> child)
    EdgeList treeEdges;
    for (int v = 1; v <= nVertices_; ++v) {
        if (parent[v] != 0) {
            Edge e;
            e.u = parent[v];
            e.v = v;
            e.type = 0; // not relevant here
            treeEdges.push_back(e);
        }
    }
    return treeEdges;
}

std::vector<EdgeList> BFSSnapshotStore::reconstructAllFullTrees() const {
    std::vector<EdgeList> all;
    if (snapshotCount() == 0) return all;

    std::vector<int> parent = initialParent_;
    for (int t = 0; t < snapshotCount(); ++t) {
        for (const auto& be : changeLists_[t]) {
            parent[be.v] = be.parent;
        }
        // build EdgeList
        EdgeList tree;
        for (int v = 1; v <= nVertices_; ++v) {
            if (parent[v] != 0) {
                Edge e;
                e.u = parent[v];
                e.v = v;
                e.type = 0;
                tree.push_back(e);
            }
        }
        all.push_back(std::move(tree));
    }
    return all;
}
