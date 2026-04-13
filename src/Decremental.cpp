#include "../include/Decremental.h"
#include "../include/Preprocess.h"
#include "../include/utils.h"
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

    m_snapshots = preprocessDecremental(numVertices, source,
                                        initialEdges, predictedUpdates);
}

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

    int i = m_lastMatched;
    EdgeList batch(m_realHistory.begin() + i,
                   m_realHistory.end());

    BFSState ws = buildWorkingState(m_snapshots[i],
                                    m_realGraph,
                                    batch);

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

void batchDeleteEdge(BFSState& ws, const EdgeList& batch)
{
    int n = ws.n;
    std::vector<std::set<int>> LL(n + 2);
    int lStar = n + 1;

    for (const auto& e : batch)
    {
        if (e.type != UpdateType::DELETE) continue;

        ws.outAdj[e.u].erase(e.v);
        ws.inAdj[e.v].erase(e.u);

        if (!ws.UP[e.v].count(e.u)) continue;
        ws.UP[e.v].erase(e.u);

        if (ws.UP[e.v].empty())
        {
            if (ws.level[e.v] != INF_LEVEL)
            {
                LL[ws.level[e.v]].insert(e.v);
                lStar = std::min(lStar, ws.level[e.v]);
            }
        }
        else if (ws.parent[e.v] == e.u)
        {
            ws.parent[e.v] = *ws.UP[e.v].begin();
        }
    }

    for (int l = lStar; l <= n; ++l)
    {
        repairLevel(ws, LL, l);
    }
}
