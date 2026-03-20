// ============================================================
// Decremental.cpp
// ============================================================

#include "../include/Decremental.h"
#include "../include/Preprocess.h"
#include "../include/BFSAlgorithms.h"
#include <algorithm>

DecrementalBFS::DecrementalBFS(int numVertices,
                               int source,
                               const EdgeList& initialEdges,
                               const EdgeList& predictedUpdates)
    : m_n(numVertices)
    , m_source(source)
    , m_predictedUpdates(predictedUpdates)
    , m_realGraph(numVertices, source)
{
    for (const auto& e : initialEdges)
        m_realGraph.addEdge(e.u, e.v);
    m_realGraph.computeBFS();
    m_realGraph.computeAllUP();

    // Preprocessing: O(m²) — stores level, parent and UP for every step
    m_snapshots = preprocessDecremental(numVertices, source,
                                        initialEdges, predictedUpdates);
}

// -----------------------------------------------------------
QueryResult DecrementalBFS::processUpdate(int step,
                                          const EdgeUpdate& realUpdate)
{
    if (realUpdate.type == UpdateType::DELETE)
        m_realGraph.removeEdge(realUpdate.u, realUpdate.v);

    m_realHistory.push_back(realUpdate);

    QueryResult result;
    result.step = step;

    int nextPred = m_lastMatched + 1;
    bool sequenceMatch =
        step == m_lastMatched + 1 &&
        nextPred <= (int)m_predictedUpdates.size() &&
        realUpdate == m_predictedUpdates[nextPred - 1];

    if (sequenceMatch) {
        m_lastMatched = nextPred;
        const auto& snap = m_snapshots[m_lastMatched];
        result.level           = snap.level;
        result.parent          = snap.parent;
        result.usedPrediction  = true;
        result.lastMatchedStep = m_lastMatched;
        return result;
    }

    // Case 2: batch repair from snapshot[i]
    int i = m_lastMatched;
    EdgeList batch(m_realHistory.begin() + i,
                   m_realHistory.end());

    BFSState ws = buildWorkingState(m_snapshots[i],
                                    m_realGraph,
                                    batch);

    // Separate the batch into deletions only (incremental setting
    // guarantees this, but we are defensive here)
    EdgeList delBatch;
    for (const auto& e : batch)
        if (e.type == UpdateType::DELETE)
            delBatch.push_back(e);

    batchDeleteEdge(ws, delBatch);

    result.level           = ws.level;
    result.parent          = ws.parent;
    result.usedPrediction  = false;
    result.lastMatchedStep = m_lastMatched;
    return result;
}
