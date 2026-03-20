// ============================================================
// Incremental.cpp
// ============================================================

#include "../include/Incremental.h"
#include "../include/Preprocess.h"
#include "../include/BFSAlgorithms.h"

IncrementalBFS::IncrementalBFS(int numVertices,
                               int source,
                               const EdgeList& initialEdges,
                               const EdgeList& predictedUpdates)
    : m_n(numVertices)
    , m_source(source)
    , m_predictedUpdates(predictedUpdates)
    , m_realGraph(numVertices, source)
{
    // Build the real graph's initial adjacency
    for (const auto& e : initialEdges)
        m_realGraph.addEdge(e.u, e.v);
    m_realGraph.computeBFS();

    // Offline preprocessing: build all predicted snapshots
    m_snapshots = preprocessIncremental(numVertices, source,
                                        initialEdges, predictedUpdates);
}

// -----------------------------------------------------------
QueryResult IncrementalBFS::processUpdate(int step,
                                          const EdgeUpdate& realUpdate)
{
    // Apply the real update to the real graph adjacency
    if (realUpdate.type == UpdateType::INSERT)
        m_realGraph.addEdge(realUpdate.u, realUpdate.v);
    // (deletions ignored in a pure incremental setting)

    m_realHistory.push_back(realUpdate);

    QueryResult result;
    result.step = step;

    int nextPred = m_lastMatched + 1;
    bool sequenceMatch =
        step == m_lastMatched + 1 &&
        nextPred <= (int)m_predictedUpdates.size() &&
        realUpdate == m_predictedUpdates[nextPred - 1];

    if (sequenceMatch) {
        // ── Case 1: sequences agree, use precomputed snapshot ──
        m_lastMatched = nextPred;
        const auto& snap = m_snapshots[m_lastMatched];
        result.level            = snap.level;
        result.parent           = snap.parent;
        result.usedPrediction   = true;
        result.lastMatchedStep  = m_lastMatched;
        return result;
    }

    // ── Case 2: diverged, run batchInsertEdge from snapshot[i] ──
    // Build a working state whose adjacency = G_i and whose
    // BFS data comes from snapshot[m_lastMatched].
    int i = m_lastMatched;
    EdgeList batch(m_realHistory.begin() + i,
                   m_realHistory.end());

    BFSState ws = buildWorkingState(m_snapshots[i],
                                    m_realGraph,
                                    batch);
    batchInsertEdge(ws, batch);

    result.level           = ws.level;
    result.parent          = ws.parent;
    result.usedPrediction  = false;
    result.lastMatchedStep = m_lastMatched;
    return result;
    // ws is discarded here — no permanent rollback needed.
}
