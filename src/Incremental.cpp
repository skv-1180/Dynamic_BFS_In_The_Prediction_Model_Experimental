#include "../include/Incremental.h"

#include "../include/utils.h"
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

void batchInsertEdge(BFSState& ws, const EdgeList& batch)
{
    int n = ws.n;
    std::vector<std::vector<int>> LR(n + 2);
    std::vector<bool> vis(n + 1, false);
    int lStar = n + 1;

    for (const auto& e : batch)
    {
        if (e.type != UpdateType::INSERT) continue;

        ws.outAdj[e.u].insert(e.v);
        ws.inAdj[e.v].insert(e.u);

        if (ws.level[e.u] == INF_LEVEL) continue;
        if (ws.level[e.v] > ws.level[e.u] + 1)
        {
            ws.parent[e.v] = e.u;
            ws.level[e.v] = ws.level[e.u] + 1;
            LR[ws.level[e.v]].push_back(e.v);
            lStar = std::min(lStar, ws.level[e.v]);
        }
    }

    if (lStar > n) return;

    for (int l = lStar; l <= n; ++l)
    {
        for (int idx = 0; idx < (int)LR[l].size(); ++idx)
        {
            int y = LR[l][idx];
            if (vis[y]) continue;
            vis[y] = true;

            for (int z : ws.outAdj[y])
            {
                if (ws.level[z] > ws.level[y] + 1)
                {
                    ws.level[z] = ws.level[y] + 1;
                    ws.parent[z] = y;
                    LR[ws.level[z]].push_back(z);
                }
            }
        }
    }
}
