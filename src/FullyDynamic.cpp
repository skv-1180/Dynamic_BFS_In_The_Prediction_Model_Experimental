#include "../include/FullyDynamic.h"

#include "../include/utils.h"
#include "../include/Preprocess.h"

FullyDynamicBFS::FullyDynamicBFS(int numVertices, int source, 
    const EdgeList& initialEdges, const EdgeList& predictedUpdates)
    : m_n(numVertices), m_source(source), m_predictedUpdates(predictedUpdates), m_realGraph(numVertices, source) 
{
    for (const auto& e : initialEdges)
        m_realGraph.addEdge(e.u, e.v);
    m_realGraph.computeBFS();
    m_realGraph.computeAllUP();

    m_snapshots = preprocessFullyDynamic(numVertices, source,
                                         initialEdges, predictedUpdates);
}

int FullyDynamicBFS::lastMatchedStep() const { return m_lastMatched; }
const BFSState& FullyDynamicBFS::realGraph() const { return m_realGraph; }
int FullyDynamicBFS::numVertices() const { return m_n; }
int FullyDynamicBFS::source() const { return m_source; }

QueryResult FullyDynamicBFS::processUpdate(int step, const EdgeUpdate& realUpdate) {
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

void batchDynamicUpdate(BFSState& ws,
                        const EdgeList& deletes,
                        const EdgeList& inserts)
{
    int n = ws.n;
    std::vector<std::vector<int>> LR(n + 2);
    std::vector<std::set<int>> LL(n + 2);
    int lStar = n + 1;

    for (const auto& e : deletes)
    {
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

    for (const auto& e : inserts)
    {
        ws.outAdj[e.u].insert(e.v);
        ws.inAdj[e.v].insert(e.u);

        if (ws.level[e.u] == INF_LEVEL) continue;

        if (ws.level[e.v] > ws.level[e.u] + 1)
        {
            ws.parent[e.v] = e.u;
            ws.level[e.v] = ws.level[e.u] + 1;
            ws.UP[e.v] = {e.u};
            LR[ws.level[e.v]].push_back(e.v);
            lStar = std::min(lStar, ws.level[e.v]);
        }
        else if (ws.level[e.v] == ws.level[e.u] + 1)
        {
            ws.UP[e.v].insert(e.u);
        }
    }

    if (lStar > n) return;

    for (int l = lStar; l <= n; ++l)
    {
        repairLevel(ws, LL, l);

        for (int idx = 0; idx < (int)LR[l].size(); ++idx)
        {
            int y = LR[l][idx];
            for (int z : ws.outAdj[y])
            {
                if (LL[l + 1].count(z))
                {
                    LL[l + 1].erase(z);
                    ws.parent[z] = y;
                    ws.level[z] = l + 1;
                    ws.UP[z].insert(y);
                    continue;
                }
                if (ws.level[z] > ws.level[y] + 1)
                {
                    ws.level[z] = ws.level[y] + 1;
                    ws.parent[z] = y;
                    ws.UP[z] = {y};
                    LR[ws.level[z]].push_back(z);
                }
                else if (ws.level[z] == ws.level[y] + 1)
                {
                    ws.UP[z].insert(y);
                }
            }
        }
    }
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
