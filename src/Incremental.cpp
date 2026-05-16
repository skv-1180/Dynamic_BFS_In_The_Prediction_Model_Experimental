#include "../include/Incremental.h"
#include "../include/Preprocess.h"
#include "../include/utils.h"
#include "../include/Config.h"

static void singleInsert(BFSState& s, int x, int y)
{
    s.addEdge(x, y);
    if (s.level[x] == INF_LEVEL) return;
    if (s.level[y] <= s.level[x] + 1) return;

    int n = s.n;
    s.parent[y] = x;
    s.level[y] = s.level[x] + 1;

    std::vector<std::vector<int>> L(n + 2);
    L[s.level[y]].push_back(y);

    for (int i = s.level[y]; i <= n; ++i)
    {
        if (L[i].empty()) break;
        for (int a : L[i])
        {
            for (int v : s.outAdj[a])
            {  // N(a) = out-neighbours of a
                if (s.level[v] > s.level[a] + 1)
                {
                    s.parent[v] = a;
                    s.level[v] = s.level[a] + 1;
                    L[s.level[v]].push_back(v);
                }
            }
        }
    }
}


IncrementalBFS::IncrementalBFS(int n, int src, const EdgeList& init, const EdgeList& pred, ErrorCorrectionMode errorCorrMode, bool useBatch)
    : m_n(n), m_source(src), m_predictedUpdates(pred),  m_errorCorrMode(errorCorrMode), prevIdx(0), prevOutList(n + 1), batch_update{useBatch}, currState{n, src, AlgorithmMode::INCREMENTAL}
{
    initGraphs(init);
    buildPredEdgeIdx(predEdgeIdx, m_predictedUpdates);
    // m_initialEdges = init;
    m_snapshots = preprocessIncremental(n, src, init, pred);

}

void IncrementalBFS::initGraphs(const EdgeList& initialEdges)
{
    
    if(!batch_update) {
        for (const auto& e : initialEdges){
            currState.addEdge(e.u, e.v);
        }
        // return;
    }

    for (const auto& e : initialEdges)
    {
        prevOutList[e.u].insert(e.v);
    }

}
 
BFSSnapshot IncrementalBFS::snapshotAt(int idx) const 
{ 
    return m_snapshots[idx]; 
    // BFSState state(m_n, m_source);
    // for (const auto& e : m_initialEdges) {
    //     state.addEdge(e.u, e.v);
    // }
    
    // for(int i=0;i<idx;i++){
    //     auto e = m_realHistory[i];
    //     state.addEdge(e.u, e.v); // as it is delete only
    // }
    // state.computeBFS();
    // return state.saveSnapshot(false);
}


QueryResult IncrementalBFS::processUpdate(int step, const EdgeUpdate& realUpdate, Timer& timer)
{
    if(!batch_update){
        return processUpdateFromPrev(step, realUpdate, timer);
    }

    if(m_errorCorrMode == ErrorCorrectionMode::TRIVIAL){
        return processUpdateTrivial(step, realUpdate, timer);
    }else{
        return processUpdateNonTrivial(step, realUpdate, timer);
    }
}

// verify 0- indexing for m_realHistory and m_predictedUpdates
QueryResult IncrementalBFS::processUpdateFromPrev(int step, const EdgeUpdate& realUpdate, Timer& timer)
{
    // timer.play();
    
    if (predEdgeIdx.count(realUpdate))
    {
        maxUpdateIdx = max(maxUpdateIdx, predEdgeIdx[realUpdate]);
    }
    else
    {
        maxUpdateIdx = -1;  // Not found
    }
    // timer.pause();


    QueryResult result; // should not be included in time
    result.step = step;
    if (maxUpdateIdx == step) // Case 1: sequence match
    {
        const auto& snap = snapshotAt(step);
        currState.addEdge(realUpdate.u, realUpdate.v);
        currState.level = snap.level;
        currState.parent = snap.parent;
        // Below part will not be calculate in time
        m_lastMatched = maxUpdateIdx;
        result.level = currState.level;
        result.parent = currState.parent;
        result.usedPrediction = true;
        result.lastMatchedStep = m_lastMatched;
        return result;
    }


    timer.play();
    singleInsert(currState, realUpdate.u, realUpdate.v);
    timer.pause();

    // Below part will not be calculate in time
    result.level = currState.level;
    result.parent = currState.parent;
    result.usedPrediction = false;
    result.lastMatchedStep = m_lastMatched;
    return result;
}

// verify 0- indexing for m_realHistory and m_predictedUpdates
QueryResult IncrementalBFS::processUpdateTrivial(int step, const EdgeUpdate& realUpdate, Timer& timer)
{
    timer.play();
    m_realHistory.push_back(realUpdate);
    
    // Doing Trivial Error correction for now
    if (predEdgeIdx.count(realUpdate))
    {
        maxUpdateIdx = max(maxUpdateIdx, predEdgeIdx[realUpdate]);
    }
    else
    {
        maxUpdateIdx = -1;  // Not found
    }
    timer.pause();

    QueryResult result; // should not be included in time
    result.step = step;
    if (maxUpdateIdx == step) // Case 1: sequence match
    {
        // Below part will not be calculate in time
        m_lastMatched = maxUpdateIdx;
        const auto& snap = snapshotAt(m_lastMatched);
        result.level = snap.level;
        result.parent = snap.parent;
        result.usedPrediction = true;
        result.lastMatchedStep = m_lastMatched;
        return result;
    }

    timer.play();
    // Case 2
    for (int k = prevIdx + 1; k <= m_lastMatched; ++k){
        const auto& e = m_realHistory[k-1];
        prevOutList[e.u].insert(e.v);
    }
    prevIdx = m_lastMatched;
    EdgeList E_ins; //can simply reuse m_realHist by passing index
    for (int k = m_lastMatched+ 1; k <= step; ++k){
        E_ins.push_back(m_realHistory[k-1]);
    }

    timer.pause();
    const auto& snap = snapshotAt(m_lastMatched);
    std::vector<int>level = snap.level;
    std::vector<int>parent = snap.parent;
    timer.play();

    batchInsertEdge(E_ins, level, parent);
    rollback(E_ins);

    timer.pause();

    // Below part will not be calculate in time
    result.level = level;
    result.parent = parent;
    result.usedPrediction = false;
    result.lastMatchedStep = m_lastMatched;
    return result;
}

// Non trivial Error correction
QueryResult IncrementalBFS::processUpdateNonTrivial(int step, const EdgeUpdate& realUpdate, Timer& timer)
{
    timer.play();
    m_realHistory.push_back(realUpdate);
    
    // Doing Trivial Error correction for now
    if (predEdgeIdx.count(realUpdate))
    {
        maxUpdateIdx = max(maxUpdateIdx, predEdgeIdx[realUpdate]);
    }
    else
    {
        maxUpdateIdx = -1;  // Not found
    }
    timer.pause();

    QueryResult result; // should not be included in time
    result.step = step;
    if (maxUpdateIdx == step) // Case 1: sequence match
    {
        // Below part will not be calculate in time
        m_lastMatched = maxUpdateIdx;
        const auto& snap = snapshotAt(m_lastMatched);
        result.level = snap.level;
        result.parent = snap.parent;
        result.usedPrediction = true;
        result.lastMatchedStep = m_lastMatched;
        return result;
    }

    timer.play();

    // Case 2 
    for (int k = prevIdx + 1; k <= m_lastMatched; ++k){
        const auto& e = m_realHistory[k-1];
        prevOutList[e.u].insert(e.v);
    }
    prevIdx = m_lastMatched;

    std::unordered_set<EdgeUpdate, EdgeUpdateHash> NonTrivialE_ins; 
    EdgeList ExtraEdges;
    for (int k = m_lastMatched+ 1; k <= step; ++k){
        NonTrivialE_ins.insert(m_realHistory[k-1]);
    }

    // Non-trivial
    int nonTrivialLastMatch = m_lastMatched;
    for (int k = m_lastMatched+ 1; k <= step; ++k) {
        const auto& pred = m_predictedUpdates[k-1];
        if (NonTrivialE_ins.count(pred)){
            NonTrivialE_ins.erase(pred);
            nonTrivialLastMatch = k;
            prevOutList[pred.u].insert(pred.v);
            ExtraEdges.push_back(pred);
        }else{
            break;
        }
    }
    EdgeList E_ins;
    for(auto it: NonTrivialE_ins){
        E_ins.push_back(it);
    }

    timer.pause();
    const auto& snap = snapshotAt(nonTrivialLastMatch);
    std::vector<int>level = snap.level;
    std::vector<int>parent = snap.parent;
    timer.play();

    batchInsertEdge(E_ins, level, parent);
    rollback(E_ins);
    rollback(ExtraEdges);
    timer.pause();

    // Below part will not be calculate in time
    result.level = level;
    result.parent = parent;
    result.usedPrediction = false;
    result.lastMatchedStep = nonTrivialLastMatch;
    return result;
}

void IncrementalBFS::batchInsertEdge(const EdgeList& batch, vector<int>&level, vector<int>&parent){
    int n = m_n;
    std::vector<std::vector<int>> LR(n + 2);
    std::vector<bool> vis(n + 1, false);
    int lStar = n + 1;

    for (const auto& e : batch)
    {
        prevOutList[e.u].insert(e.v);
        if (level[e.u] == INF_LEVEL) continue;
        if (level[e.v] > level[e.u] + 1)
        {
            parent[e.v] = e.u;
            level[e.v] = level[e.u] + 1;
            LR[level[e.v]].push_back(e.v);
            lStar = std::min(lStar, level[e.v]);
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

            for (int z : prevOutList[y])
            {
                if (level[z] > level[y] + 1)
                {
                    level[z] = level[y] + 1;
                    parent[z] = y;
                    LR[level[z]].push_back(z);
                }
            }
        }
    }
}

void IncrementalBFS::rollback(const EdgeList& batch){
    for (const auto& e : batch)
    {
        prevOutList[e.u].erase(e.v);
    }
}

