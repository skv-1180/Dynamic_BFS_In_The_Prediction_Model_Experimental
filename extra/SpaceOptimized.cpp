// ============================================================
// SpaceOptimized.cpp  –  §6 Space-Efficient Incremental BFS
//
// Bug fixes vs previous version:
//   1. Removed m_realGraph.computeBFS() — real graph only tracks adjacency.
//   2. Added m_initGraph (G_0) for correct G_i reconstruction in Case 2.
//   3. Case 2: G_i is rebuilt by replaying G_0→G_i (like §3 Incremental)
//      instead of the broken undo-in-reverse buildWorkingState.
//   4. Added step == m_lastMatched+1 guard on sequence match.
// ============================================================
#include "../include/SpaceOptimized.h"
#include "../include/BFSAlgorithms.h"
#include <algorithm>

SpaceOptimizedIncremental::SpaceOptimizedIncremental(
    int numVertices, int source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates)
    : m_n(numVertices)
    , m_source(source)
    , m_numPredicted((int)predictedUpdates.size())
    , m_initLevel(numVertices + 1, INF_LEVEL)
    , m_initParent(numVertices + 1, NO_PARENT)
    , m_changes(numVertices + 1)
    , m_predictedUpdates(predictedUpdates)
    , m_initGraph(numVertices, source)
    , m_realGraph(numVertices, source)
{
    // Build G_0
    BFSState state(numVertices, source);
    for (const auto& e : initialEdges) {
        state.addEdge(e.u, e.v);
        m_initGraph.addEdge(e.u, e.v);
        m_realGraph.addEdge(e.u, e.v);
    }
    state.computeBFS();
    // NOTE: m_realGraph does NOT get computeBFS() — it tracks adjacency only.

    m_initLevel  = state.level;
    m_initParent = state.parent;

    // Simulate predicted insertions, record per-vertex changes
    for (int i = 0; i < m_numPredicted; ++i) {
        const auto& e = predictedUpdates[i];
        if (e.type != UpdateType::INSERT) continue;

        std::vector<int> oldLevel  = state.level;
        std::vector<int> oldParent = state.parent;

        classicalInsertEdge(state, e.u, e.v);

        for (int v = 1; v <= numVertices; ++v) {
            if (state.level[v] != oldLevel[v] || state.parent[v] != oldParent[v])
                m_changes[v].push_back({i + 1, state.level[v], state.parent[v]});
        }
    }
}

int SpaceOptimizedIncremental::getLevelAt(int v, int j) const
{
    const auto& cl = m_changes[v];
    auto it = std::upper_bound(cl.begin(), cl.end(), j,
        [](int val, const ChangeEntry& ce){ return val < ce.step; });
    if (it == cl.begin()) return m_initLevel[v];
    return (--it)->level;
}

int SpaceOptimizedIncremental::getParentAt(int v, int j) const
{
    const auto& cl = m_changes[v];
    auto it = std::upper_bound(cl.begin(), cl.end(), j,
        [](int val, const ChangeEntry& ce){ return val < ce.step; });
    if (it == cl.begin()) return m_initParent[v];
    return (--it)->parent;
}

BFSSnapshot SpaceOptimizedIncremental::getSnapshotAt(int j) const
{
    BFSSnapshot snap;
    snap.level.resize(m_n + 1);
    snap.parent.resize(m_n + 1);
    snap.hasUpperParents = false;
    for (int v = 1; v <= m_n; ++v) {
        snap.level[v]  = getLevelAt(v, j);
        snap.parent[v] = getParentAt(v, j);
    }
    return snap;
}

QueryResult SpaceOptimizedIncremental::processUpdate(
    int step, const EdgeUpdate& realUpdate)
{
    if (realUpdate.type == UpdateType::INSERT)
        m_realGraph.addEdge(realUpdate.u, realUpdate.v);
    m_realHistory.push_back(realUpdate);

    QueryResult result;
    result.step = step;

    int nextPred = m_lastMatched + 1;
    // Require consecutive match (step == lastMatched+1) to ensure G_i = Ĝ_i
    bool sequenceMatch =
        (step == m_lastMatched + 1) &&
        (nextPred <= m_numPredicted) &&
        (realUpdate == m_predictedUpdates[nextPred - 1]);

    if (sequenceMatch) {
        // Case 1: serve from change-list snapshot.  O(n log n).
        m_lastMatched = nextPred;
        BFSSnapshot snap = getSnapshotAt(m_lastMatched);
        result.level           = snap.level;
        result.parent          = snap.parent;
        result.usedPrediction  = true;
        result.lastMatchedStep = m_lastMatched;
        return result;
    }

    // Case 2: reconstruct G_i from G_0 by replaying real updates 0..i-1,
    // then compute E_ins = edges in G_j \ G_i, run batchInsertEdge.
    int i = m_lastMatched;

    // Reconstruct G_i adjacency
    BFSState Gi(m_n, m_source);
    Gi.outAdj = m_initGraph.outAdj;
    Gi.inAdj  = m_initGraph.inAdj;
    for (int k = 0; k < i; ++k) {
        const auto& e = m_realHistory[k];
        if (e.type == UpdateType::INSERT) {
            Gi.outAdj[e.u].insert(e.v);
            Gi.inAdj[e.v].insert(e.u);
        }
    }

    // E_ins = net insertions from G_i to G_j
    EdgeList E_ins;
    for (int u = 1; u <= m_n; ++u)
        for (int v : m_realGraph.outAdj[u])
            if (!Gi.outAdj[u].count(v))
                E_ins.push_back({u, v, UpdateType::INSERT});

    // Build working state: G_i adjacency + T̂_i BFS data
    BFSSnapshot snap = getSnapshotAt(i);
    BFSState ws(m_n, m_source);
    ws.outAdj = Gi.outAdj;
    ws.inAdj  = Gi.inAdj;
    ws.loadSnapshot(snap);

    batchInsertEdge(ws, E_ins);

    result.level           = ws.level;
    result.parent          = ws.parent;
    result.usedPrediction  = false;
    result.lastMatchedStep = m_lastMatched;
    return result;
}
