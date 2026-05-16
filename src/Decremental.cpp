#include "../include/Decremental.h"
#include "../include/Preprocess.h"
#include "../include/utils.h"

DecrementalBFS::DecrementalBFS(int n, int src, const EdgeList& init, const EdgeList& pred, ErrorCorrectionMode errorCorrMode)
    : m_n(n), m_source(src), m_predictedUpdates(pred), m_errorCorrMode(errorCorrMode), prevIdx{0}, prevInList(n + 1), prevOutList(n + 1)
{
    currEdgeCnt = init.size();
    initGraphs(init);
    buildPredEdgeIdx(predEdgeIdx, m_predictedUpdates);
    // m_initialEdges = init;
    m_snapshots = preprocessDecremental(n, src, init, pred);

}

void DecrementalBFS::initGraphs(const EdgeList& initialEdges)
{
    for (const auto& e : initialEdges) {
        prevOutList[e.u].insert(e.v);
        prevInList[e.v].insert(e.u);
    }
}

BFSSnapshot DecrementalBFS::snapshotAt(int idx) const 
{   
    return m_snapshots[idx]; 
    // BFSState state(m_n, m_source);
    // for (const auto& e : m_initialEdges) {
    //     state.addEdge(e.u, e.v);
    // }
    
    // for(int i=0;i<idx;i++){
    //     auto e = m_realHistory[i];
    //     state.removeEdge(e.u, e.v); // as it is delete only
    // }
    // state.computeBFS();
    // state.computeAllUP();
    // return state.saveSnapshot(true);
}


QueryResult DecrementalBFS::processUpdate(int step, const EdgeUpdate& realUpdate, Timer& timer){
    if(m_errorCorrMode == ErrorCorrectionMode::TRIVIAL){
        return processUpdateTrivial(step, realUpdate, timer);
    }else{
        return processUpdateNonTrivial(step, realUpdate, timer);
    }
}

QueryResult DecrementalBFS::processUpdateTrivial(int step, const EdgeUpdate& realUpdate, Timer& timer)
{
    timer.play();
    m_realHistory.push_back(realUpdate);
    if(realUpdate.type == UpdateType::DELETE){
        --currEdgeCnt;
    }else{
        ++currEdgeCnt;
    }

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

    QueryResult result;
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
        prevOutList[e.u].erase(e.v);
        prevInList[e.v].erase(e.u);
    }
    prevIdx = m_lastMatched;
    
    EdgeList E_del; //can simply reuse m_realHist by passing index
    for (int k = m_lastMatched+ 1; k <= step; ++k){
        E_del.push_back(m_realHistory[k-1]);
    }

    timer.pause();
    const auto& snap = snapshotAt(m_lastMatched);
    std::vector<int>level = snap.level;
    std::vector<int>parent = snap.parent;
    std::vector<std::unordered_set<int>> UP = snap.upperParents;
    timer.play();

    batchDeleteEdge(E_del, level, parent, UP);
    rollback(E_del);

    timer.pause();

    // Below part will not be calculate in time
    result.level = level;
    result.parent = parent;
    result.usedPrediction = false;
    result.lastMatchedStep = m_lastMatched;
    return result;
}

QueryResult DecrementalBFS::processUpdateNonTrivial(int step, const EdgeUpdate& realUpdate, Timer& timer)
{
    timer.play();
    m_realHistory.push_back(realUpdate);
    if(realUpdate.type == UpdateType::DELETE){
        --currEdgeCnt;
    }else{
        ++currEdgeCnt;
    }

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

    QueryResult result;
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
        prevOutList[e.u].erase(e.v);
        prevInList[e.v].erase(e.u);
    }
    prevIdx = m_lastMatched;

    std::unordered_set<EdgeUpdate, EdgeUpdateHash> NonTrivialE_del; 
    EdgeList ExtraEdges; 
    for (int k = m_lastMatched+ 1; k <= step; ++k){
        NonTrivialE_del.insert(m_realHistory[k-1]);
    }

    // Non-trivial
    int nonTrivialLastMatch = m_lastMatched;
    for (int k = m_lastMatched+ 1; k <= step; ++k) {
        const auto& pred = m_predictedUpdates[k-1];
        if (NonTrivialE_del.count(pred)){
            NonTrivialE_del.erase(pred);
            nonTrivialLastMatch = k;
            prevOutList[pred.u].erase(pred.v);
            prevInList[pred.v].erase(pred.u);
            ExtraEdges.push_back(pred);
        }else{
            break;
        }
    }
    
    EdgeList E_del; //can simply reuse m_realHist by passing index
    for(auto it: NonTrivialE_del){
        E_del.push_back(it);
    }

    timer.pause();
    const auto& snap = snapshotAt(nonTrivialLastMatch);
    std::vector<int>level = snap.level;
    std::vector<int>parent = snap.parent;
    std::vector<std::unordered_set<int>> UP = snap.upperParents;
    timer.play();

    batchDeleteEdge(E_del, level, parent, UP);
    rollback(E_del);
    rollback(ExtraEdges);

    timer.pause();

    // Below part will not be calculate in time
    result.level = level;
    result.parent = parent;
    result.usedPrediction = false;
    result.lastMatchedStep = m_lastMatched;
    return result;
}

void DecrementalBFS::batchDeleteEdge(const EdgeList& batch, vector<int>&level, vector<int>&parent, std::vector<std::unordered_set<int>>& UP)
{
    int n = m_n;
    std::vector<std::unordered_set<int>> LL(n + 2); // change to vector
    int lStar = n + 1;

    for (const auto& e : batch)
    {
        prevOutList[e.u].erase(e.v);
        prevInList[e.v].erase(e.u);

        if (!UP[e.v].count(e.u)) continue;
        UP[e.v].erase(e.u);

        if (UP[e.v].empty())
        {
            if (level[e.v] != INF_LEVEL)
            {
                LL[level[e.v]].insert(e.v);
                lStar = std::min(lStar, level[e.v]);
            }
        }
        else if (parent[e.v] == e.u)
        {
            parent[e.v] = *UP[e.v].begin();
        }
    }

    int cnt = 0;

    for (int l = lStar; l <= n; ++l)
    {
        repairLevel(LL, l, level, parent, UP, n, prevInList, prevOutList, cnt);
        LL[l].clear();
        if(cnt > (currEdgeCnt/2)){
            cnt = -1;
            break;
        }
    }
    if(cnt == -1){
        fallback_BFS(m_source, m_n, level, parent, prevOutList);
    }
}

void DecrementalBFS::rollback(const EdgeList& batch) {
    for (const auto& e : batch)
    {
        prevOutList[e.u].insert(e.v);
        prevInList[e.v].insert(e.u);
    }
}
