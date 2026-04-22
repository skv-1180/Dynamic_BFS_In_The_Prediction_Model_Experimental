// ============================================================
// SpaceOptimizedDecremental.cpp
// Paper §new: Space-Efficient Decremental BFS with Predictions.
//
// Core idea: replace O(m²) UP-set storage with pointer-based
// checkpoints at O(mn/k) total space, checkpoint interval k.
// ============================================================
#include "../include/SpaceOptimizedDecremental.h"
#include "../include/BFSAlgorithms.h"
#include "../include/Preprocess.h"
#include <algorithm>
#include <queue>

// ── Constructor ──────────────────────────────────────────────
SpaceOptimizedDecremental::SpaceOptimizedDecremental(
    int numVertices, int source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates,
    int k)
    : m_n(numVertices)
    , m_source(source)
    , m_k(std::max(1, k))
    , m_numPredicted((int)predictedUpdates.size())
    , m_predictedUpdates(predictedUpdates)
    , m_initGraph(numVertices, source)
    , m_realGraph(numVertices, source)
    , m_inAdj0_sorted(numVertices + 1)
{
    // Build G_0 adjacency
    for (const auto& e : initialEdges) {
        m_initGraph.addEdge(e.u, e.v);
        m_realGraph.addEdge(e.u, e.v);
    }

    // Build sorted in-adjacency for cursor-based replay
    for (int v = 1; v <= m_n; ++v) {
        m_inAdj0_sorted[v].assign(
            m_initGraph.inAdj[v].begin(),
            m_initGraph.inAdj[v].end());
        std::sort(m_inAdj0_sorted[v].begin(),
                  m_inAdj0_sorted[v].end());
    }

    // ── Offline preprocessing: simulate predicted deletions ──
    // Run a BFS simulation, recording checkpoints every k steps.
    // We maintain:
    //   simLevel[v], simParent[v]: current BFS tree
    //   simPtr[v]: cursor into simInAdj[v] (sorted in-neighbours)
    //   simInAdj[v]: current sorted in-neighbour list (shrinks with deletions)

    std::vector<int> simLevel(m_n + 1, INF_LEVEL);
    std::vector<int> simParent(m_n + 1, NO_PARENT);
    // ptr[v] = index of first unexamined element in simInAdj[v]
    std::vector<int> simPtr(m_n + 1, 0);
    // sorted in-adjacency, updated as deletions happen
    std::vector<std::vector<int>> simIn(m_n + 1);
    for (int v = 1; v <= m_n; ++v) {
        simIn[v] = m_inAdj0_sorted[v];
    }

    // Initial BFS of G_0
    {
        std::fill(simLevel.begin(), simLevel.end(), INF_LEVEL);
        simLevel[m_source] = 0; simParent[m_source] = NO_PARENT;
        std::queue<int> q; q.push(m_source);
        BFSState tmp(m_n, m_source);
        tmp.outAdj = m_initGraph.outAdj;
        tmp.inAdj  = m_initGraph.inAdj;
        tmp.computeBFS();
        simLevel  = tmp.level;
        simParent = tmp.parent;
        // Initial ptrs: advance past all neighbours with level != simLevel[v]-1
        // to point to first valid upper-parent (or end).
        for (int v = 1; v <= m_n; ++v) {
            simPtr[v] = 0; // start at beginning; cursor is reset on each level change
        }
    }

    // Store checkpoint at step 0
    auto storeCheckpoint = [&](int step) {
        DecrementalCheckpoint cp;
        cp.step   = step;
        cp.level  = simLevel;
        cp.parent = simParent;
        cp.ptr    = simPtr;
        m_checkpoints.push_back(std::move(cp));
    };
    storeCheckpoint(0);

    // Simulate predicted deletions
    for (int i = 0; i < m_numPredicted; ++i) {
        const auto& e = predictedUpdates[i];
        if (e.type != UpdateType::DELETE) {
            // In a pure decremental setting all predicted updates are deletions.
            // If a non-deletion appears, skip it.
            if ((i + 1) % m_k == 0) storeCheckpoint(i + 1);
            continue;
        }

        int u = e.u, v = e.v;

        // Remove u from simIn[v]
        auto& inv = simIn[v];
        inv.erase(std::remove(inv.begin(), inv.end(), u), inv.end());
        // Clamp ptr[v] to new size
        if (simPtr[v] > (int)inv.size()) simPtr[v] = (int)inv.size();

        // Is u the parent of v (i.e. does the edge lie on the BFS tree)?
        // In the pointer model: u is an "upper parent candidate" if
        // level[u] == level[v]-1. The parent is the first such found by cursor.
        // Simplified: if the deletion removes the parent edge, repair.
        if (simParent[v] != u) {
            // Not the parent edge, but might affect cursor position — no BFS change.
            if ((i + 1) % m_k == 0) storeCheckpoint(i + 1);
            continue;
        }

        // Parent edge removed. Repair using cursor-based scan.
        // Use a BFS-ordered level queue (ES-tree style) via classicalDeleteEdge
        // but track the ptr[] arrays.
        // For simplicity and correctness, we rebuild with classicalDeleteEdge
        // on a temporary BFSState that has the current simIn state, then
        // extract the updated level/parent. The pointer semantics are encoded
        // as: ptr[v] = index of v's parent in simIn[v] after repair,
        // or simIn[v].size() if v became unreachable.
        {
            BFSState tmp(m_n, m_source);
            // Reconstruct current adjacency
            for (int w = 1; w <= m_n; ++w) {
                for (int nb : simIn[w])
                    tmp.inAdj[w].insert(nb);
            }
            // Out-adjacency from in-adjacency
            for (int w = 1; w <= m_n; ++w)
                for (int nb : tmp.inAdj[w])
                    tmp.outAdj[nb].insert(w);

            tmp.level  = simLevel;
            tmp.parent = simParent;
            tmp.computeAllUP();
            classicalDeleteEdge(tmp, u, v);
            simLevel  = tmp.level;
            simParent = tmp.parent;
        }

        // Update ptrs: for each v, ptr[v] = index of parent in simIn[v]
        // (or simIn[v].size() if unreachable / no parent)
        for (int w = 1; w <= m_n; ++w) {
            simPtr[w] = 0;
            if (simParent[w] != NO_PARENT) {
                auto& inv2 = simIn[w];
                auto it = std::find(inv2.begin(), inv2.end(), simParent[w]);
                simPtr[w] = (it != inv2.end())
                    ? (int)(it - inv2.begin()) : (int)inv2.size();
            }
        }

        if ((i + 1) % m_k == 0) storeCheckpoint(i + 1);
    }

    // If the last step wasn't a checkpoint, store it anyway so we can
    // access any predicted step via getSnapshotAt.
    if (m_numPredicted % m_k != 0)
        storeCheckpoint(m_numPredicted);
}

// ── getSnapshotAt ─────────────────────────────────────────────
BFSSnapshot SpaceOptimizedDecremental::replayFromCheckpoint(
    const DecrementalCheckpoint& cp, int targetStep) const
{
    // Replay predicted deletions from cp.step to targetStep.
    // We need to rebuild simIn from m_initGraph minus deletions up to targetStep.
    std::vector<std::vector<int>> simIn(m_n + 1);
    for (int v = 1; v <= m_n; ++v)
        simIn[v] = m_inAdj0_sorted[v];

    // Apply all predicted deletions from 0 to cp.step-1 to get cp adjacency
    for (int i = 0; i < cp.step && i < m_numPredicted; ++i) {
        const auto& e = m_predictedUpdates[i];
        if (e.type == UpdateType::DELETE) {
            auto& inv = simIn[e.v];
            inv.erase(std::remove(inv.begin(), inv.end(), e.u), inv.end());
        }
    }

    // Start from checkpoint state
    std::vector<int> lev    = cp.level;
    std::vector<int> par    = cp.parent;

    // Replay from cp.step to targetStep
    for (int i = cp.step; i < targetStep && i < m_numPredicted; ++i) {
        const auto& e = m_predictedUpdates[i];
        if (e.type != UpdateType::DELETE) continue;

        int u = e.u, v = e.v;
        auto& inv = simIn[v];
        inv.erase(std::remove(inv.begin(), inv.end(), u), inv.end());

        if (par[v] != u) continue; // not the parent edge

        // Repair: full BFS recomputation on current state for simplicity
        BFSState tmp(m_n, m_source);
        for (int w = 1; w <= m_n; ++w)
            for (int nb : simIn[w]) {
                tmp.inAdj[w].insert(nb);
                tmp.outAdj[nb].insert(w);
            }
        tmp.level  = lev;
        tmp.parent = par;
        tmp.computeAllUP();
        classicalDeleteEdge(tmp, u, v);
        lev = tmp.level;
        par = tmp.parent;
    }

    BFSSnapshot snap;
    snap.level            = lev;
    snap.parent           = par;
    snap.hasUpperParents  = false;
    return snap;
}

BFSSnapshot SpaceOptimizedDecremental::getSnapshotAt(int i) const
{
    // Find the largest checkpoint step ≤ i
    // Checkpoints are stored at 0, k, 2k, ... in order.
    int cpIdx = 0;
    for (int c = (int)m_checkpoints.size() - 1; c >= 0; --c) {
        if (m_checkpoints[c].step <= i) { cpIdx = c; break; }
    }
    const auto& cp = m_checkpoints[cpIdx];
    if (cp.step == i) {
        // Exact checkpoint match
        BFSSnapshot snap;
        snap.level           = cp.level;
        snap.parent          = cp.parent;
        snap.hasUpperParents = false;
        return snap;
    }
    // Replay from checkpoint to step i
    return replayFromCheckpoint(cp, i);
}

// ── Online update ─────────────────────────────────────────────
QueryResult SpaceOptimizedDecremental::processUpdate(
    int step, const EdgeUpdate& realUpdate)
{
    if (realUpdate.type == UpdateType::DELETE)
        m_realGraph.removeEdge(realUpdate.u, realUpdate.v);
    m_realHistory.push_back(realUpdate);

    QueryResult result;
    result.step = step;

    int nextPred = m_lastMatched + 1;
    bool sequenceMatch =
        (step == m_lastMatched + 1) &&
        (nextPred <= m_numPredicted) &&
        (realUpdate == m_predictedUpdates[nextPred - 1]);

    if (sequenceMatch) {
        // ── Case 1: serve from checkpoint (O(n) or O(n·k)) ──
        m_lastMatched = nextPred;
        BFSSnapshot snap = getSnapshotAt(m_lastMatched);
        result.level           = snap.level;
        result.parent          = snap.parent;
        result.usedPrediction  = true;
        result.lastMatchedStep = m_lastMatched;
        return result;
    }

    // ── Case 2: reconstruct G_i and run batchDeleteEdge ──────
    int i = m_lastMatched;
    BFSSnapshot snap = getSnapshotAt(i);

    // Reconstruct G_i adjacency by replaying deletions from G_0
    BFSState Gi(m_n, m_source);
    Gi.outAdj = m_initGraph.outAdj;
    Gi.inAdj  = m_initGraph.inAdj;
    for (int k = 0; k < i; ++k) {
        const auto& e = m_realHistory[k];
        if (e.type == UpdateType::DELETE) {
            Gi.outAdj[e.u].erase(e.v);
            Gi.inAdj[e.v].erase(e.u);
        }
    }

    // E_del = edges in G_i (Gi) not in G_j (m_realGraph)
    EdgeList E_del;
    for (int u = 1; u <= m_n; ++u)
        for (int v : Gi.outAdj[u])
            if (!m_realGraph.outAdj[u].count(v))
                E_del.push_back({u, v, UpdateType::DELETE});

    // Build working state: G_i adjacency + T̂_i BFS data
    BFSState ws(m_n, m_source);
    ws.outAdj = Gi.outAdj;
    ws.inAdj  = Gi.inAdj;
    ws.loadSnapshot(snap);
    // Compute UP sets for batchDeleteEdge
    ws.computeAllUP();

    batchDeleteEdge(ws, E_del);

    result.level           = ws.level;
    result.parent          = ws.parent;
    result.usedPrediction  = false;
    result.lastMatchedStep = m_lastMatched;
    return result;
}
