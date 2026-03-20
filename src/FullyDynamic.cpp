#include "../include/FullyDynamic.h"

#include "../include/BFSAlgorithms.h"
#include "../include/Preprocess.h"

FullyDynamicBFS::FullyDynamicBFS(int numVertices,
                                 int source,
                                 const EdgeList& initialEdges,
                                 const EdgeList& predictedUpdates)
    : m_n(numVertices), m_source(source), m_predictedUpdates(predictedUpdates), m_realGraph(numVertices, source) {
    for (const auto& e : initialEdges)
        m_realGraph.addEdge(e.u, e.v);
    m_realGraph.computeBFS();
    m_realGraph.computeAllUP();

    m_snapshots = preprocessFullyDynamic(numVertices, source,
                                         initialEdges, predictedUpdates);
}

QueryResult FullyDynamicBFS::fallbackBFS(int step) {
    m_realGraph.computeBFS();
    QueryResult r;
    r.step = step;
    r.level = m_realGraph.level;
    r.parent = m_realGraph.parent;
    r.usedPrediction = false;
    r.lastMatchedStep = m_lastMatched;
    return r;
}

QueryResult FullyDynamicBFS::processUpdate(int step,
                                           const EdgeUpdate& realUpdate) {
    if (realUpdate.type == UpdateType::INSERT)
        m_realGraph.addEdge(realUpdate.u, realUpdate.v);
    else
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
        // Case 1
        m_lastMatched = nextPred;
        const auto& snap = m_snapshots[m_lastMatched];
        result.level = snap.level;
        result.parent = snap.parent;
        result.usedPrediction = true;
        result.lastMatchedStep = m_lastMatched;
        return result;
    }

    // Case 2
    int i = m_lastMatched;
    EdgeList batch(m_realHistory.begin() + i,
                   m_realHistory.end());

    //! fallback remaining

    BFSState ws = buildWorkingState(m_snapshots[i],
                                    m_realGraph,
                                    batch);

    EdgeList insBatch, delBatch;
    for (const auto& e : batch) {
        if (e.type == UpdateType::INSERT)
            insBatch.push_back(e);
        else
            delBatch.push_back(e);
    }

    batchDynamicUpdate(ws, delBatch, insBatch);

    result.level = ws.level;
    result.parent = ws.parent;
    result.usedPrediction = false;
    result.lastMatchedStep = m_lastMatched;
    return result;
}
