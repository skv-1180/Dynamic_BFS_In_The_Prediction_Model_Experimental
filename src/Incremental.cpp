#include "../include/Incremental.h"

#include "../include/BFSAlgorithms.h"
#include "../include/Preprocess.h"

IncrementalBFS::IncrementalBFS(int numVertices,
                               int source,
                               const EdgeList& initialEdges,
                               const EdgeList& predictedUpdates)
    : m_n(numVertices), 
    m_source(source), 
    m_predictedUpdates(predictedUpdates), 
    m_realGraph(numVertices, source) 
{
    for (const auto& e : initialEdges)
        m_realGraph.addEdge(e.u, e.v);
    m_realGraph.computeBFS();

    m_snapshots = preprocessIncremental(numVertices, source,
                                        initialEdges, predictedUpdates);
}

QueryResult IncrementalBFS::processUpdate(int step,
                                          const EdgeUpdate& realUpdate) {
    if (realUpdate.type == UpdateType::INSERT)
        m_realGraph.addEdge(realUpdate.u, realUpdate.v);

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

    BFSState ws = buildWorkingState(m_snapshots[i],
                                    m_realGraph,
                                    batch);
    batchInsertEdge(ws, batch);

    result.level = ws.level;
    result.parent = ws.parent;
    result.usedPrediction = false;
    result.lastMatchedStep = m_lastMatched;
    return result;
}
