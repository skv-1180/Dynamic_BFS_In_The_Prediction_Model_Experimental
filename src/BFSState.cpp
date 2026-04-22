#include "../include/BFSState.h"
#include <queue>
#include <cassert>


BFSState::BFSState(int n, int source, AlgorithmMode algMode)
    : n(n), source(source),
      level(n + 1, INF_LEVEL),
      parent(n + 1, NO_PARENT),
      outAdj(n + 1),
      mode(algMode)
{
    if(mode != AlgorithmMode::INCREMENTAL) {
        UP.resize(n + 1);
        inAdj.resize(n + 1);
    }
    level[source] = 0;
}

void BFSState::addEdge(int u, int v)
{
    outAdj[u].insert(v);
    if(mode != AlgorithmMode::INCREMENTAL) {
        inAdj[v].insert(u);
    }
}

void BFSState::removeEdge(int u, int v)
{
    outAdj[u].erase(v);
    if(mode != AlgorithmMode::INCREMENTAL) {
        inAdj[v].erase(u);
    }
}

void BFSState::computeBFS()
{
    // Reset
    std::fill(level.begin(),  level.end(),  INF_LEVEL);
    std::fill(parent.begin(), parent.end(), NO_PARENT);
    for (auto& s : UP) s.clear();

    level[source] = 0;
    std::queue<int> q;
    q.push(source);

    while (!q.empty()) {
        int u = q.front(); q.pop();
        for (int v : outAdj[u]) {
            if (level[v] == INF_LEVEL) {
                level[v]  = level[u] + 1;
                parent[v] = u;
                q.push(v);
            }
        }
    }
}

void BFSState::computeAllUP()
{
    for (auto& s : UP) s.clear();
    for (int v = 1; v <= n; ++v) {
        if (level[v] == INF_LEVEL) continue;
        for (int u : inAdj[v]) {
            if (level[u] != INF_LEVEL && level[u] == level[v] - 1) {
                UP[v].insert(u);
            }
        }
    }
}

BFSSnapshot BFSState::saveSnapshot(bool saveUP) const
{
    BFSSnapshot snap;
    snap.level  = level;
    snap.parent = parent;
    if (saveUP) {
        snap.upperParents   = UP;
        snap.hasUpperParents = true;
    }
    return snap;
}

void BFSState::loadSnapshot(const BFSSnapshot& snap)
{
    level  = snap.level;
    parent = snap.parent;
    if (snap.hasUpperParents) {
        UP = snap.upperParents;
    } else {
        for (auto& s : UP) s.clear();
    }
}

void BFSState::printBFSTree(std::ostream& os) const
{
    os << "BFS Tree (source=" << source << "):\n";
    os << "  Vertex | Level | Parent\n";
    os << "  -------|-------|-------\n";
    for (int v = 1; v <= n; ++v) {
        os << "  " << v << "      | ";
        if (level[v] == INF_LEVEL) os << "INF   | ";
        else                       os << level[v] << "     | ";
        if (parent[v] == NO_PARENT) os << "none\n";
        else                        os << parent[v] << "\n";
    }
}

void BFSState::printAdjacency(std::ostream& os) const
{
    os << "Adjacency (directed):\n";
    for (int u = 1; u <= n; ++u) {
        os << "  " << u << " -> {";
        bool first = true;
        for (int v : outAdj[u]) { if (!first) os << ", "; os << v; first = false; }
        os << "}\n";
    }
}

bool BFSState::verify() const
{
    // Run a fresh BFS and compare.
    BFSState tmp(n, source);
    tmp.outAdj = outAdj;
    tmp.inAdj  = inAdj;
    tmp.computeBFS();

    for (int v = 1; v <= n; ++v) {
        if (level[v] != tmp.level[v]) {
            std::cerr << "[VERIFY FAIL] vertex " << v
                      << " level=" << level[v]
                      << " expected=" << tmp.level[v] << "\n";
            return false;
        }
    }
    return true;
}
