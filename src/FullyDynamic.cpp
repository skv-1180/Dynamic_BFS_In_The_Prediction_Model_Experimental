#include "../include/FullyDynamic.h"
#include "../include/Preprocess.h"
#include "../include/utils.h"

FullyDynamicBFS::FullyDynamicBFS(int n, int src, const EdgeList& init, const EdgeList& pred, ErrorCorrectionMode errorCorrMode)
    : m_n(n), m_source(src), m_predictedUpdates(pred),m_errorCorrMode(errorCorrMode), prevIdx{0}, prevInList(n + 1), prevOutList(n + 1)
{
    currEdgeCnt = init.size();
    initRealGraph(init);
    buildPredEdgeIdx(predEdgeIdx, m_predictedUpdates);
    m_initialEdges = init;
    // m_snapshots = preprocessFullyDynamic(n, src, init, pred);
}

void FullyDynamicBFS::initRealGraph(const EdgeList& initialEdges)
{
    for (const auto& e : initialEdges) {
        prevOutList[e.u].insert(e.v);
        prevInList[e.v].insert(e.u);
    }
}

BFSSnapshot FullyDynamicBFS::snapshotAt(int idx) const { 
    // return m_snapshots[i]; 
    BFSState state(m_n, m_source);
    for (const auto& e : m_initialEdges) {
        state.addEdge(e.u, e.v);
    }
    
    for(int i=0;i<idx;i++){
        auto e = m_realHistory[i];
        if(e.type == UpdateType::DELETE){
            state.removeEdge(e.u, e.v);
        }else{
            state.addEdge(e.u, e.v);
        }
    }
    state.computeBFS();
    state.computeAllUP();
    return state.saveSnapshot(true);
}

void FullyDynamicBFS::rollback(const EdgeList& dels, const EdgeList& ins){
    for (const auto& e : dels)
    {
        prevOutList[e.u].insert(e.v);
        prevInList[e.v].insert(e.u);
    }

    for (const auto& e : ins)
    {
        prevOutList[e.u].erase(e.v);
        prevInList[e.v].erase(e.u);
    }
}

void FullyDynamicBFS::rollback(const EdgeList& extraEdges){
    for (const auto& e : extraEdges){
        if(e.type == UpdateType::DELETE){
            prevOutList[e.u].insert(e.v);
            prevInList[e.v].insert(e.u);
        }else{
            prevOutList[e.u].erase(e.v);
            prevInList[e.v].erase(e.u);
        }
    }
}

QueryResult FullyDynamicBFS::processUpdate(int step, const EdgeUpdate& realUpdate, Timer& timer)
{
    if(m_errorCorrMode == ErrorCorrectionMode::TRIVIAL){
        return processUpdateTrivial(step, realUpdate, timer);
    }else{
        return processUpdateNonTrivial(step, realUpdate, timer);
    }
}

int fall = 0;

QueryResult FullyDynamicBFS::processUpdateTrivial(int step, const EdgeUpdate& realUpdate, Timer& timer)
{
    if(step % 10000 == 0){
        std::cerr << "step: " << step << endl;
        // std::cerr << "lastMatched: " << m_lastMatched << endl;
        // std::cerr << "fallback: " << fall << endl;
    }

    timer.play();
    realHash.insert(realUpdate);
    predHash.insert(m_predictedUpdates[step-1]);

    m_realHistory.push_back(realUpdate);
    if(realUpdate.type == UpdateType::DELETE){
        --currEdgeCnt;
    }else{
        ++currEdgeCnt;
    }

    timer.pause();
   
    QueryResult result;
    result.step = step;
    if (realHash.sameSet(predHash)) // Case 1: sequence match
    {
        // Below part will not be calculate in time
        m_lastMatched = step;
        // const auto& snap = m_snapshots[m_lastMatched];
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
        if(e.type == UpdateType::DELETE){
            prevOutList[e.u].erase(e.v);
            prevInList[e.v].erase(e.u);
        }else{
            prevOutList[e.u].insert(e.v);
            prevInList[e.v].insert(e.u);
        }
    }
    prevIdx = m_lastMatched;

    EdgeList E_del, E_ins; //can simply reuse m_realHist by passing index
    for (int k = m_lastMatched+ 1; k <= step; ++k){
        if(m_realHistory[k-1].type == UpdateType::DELETE){
            E_del.push_back(m_realHistory[k-1]);
        }else{
            E_ins.push_back(m_realHistory[k-1]);
        }
    }
    removeCommonEdges(E_del, E_ins);

    timer.pause();
    const auto& snap = snapshotAt(m_lastMatched);
    std::vector<int>level = snap.level;
    std::vector<int>parent = snap.parent;
    std::vector<std::unordered_set<int>> UP = snap.upperParents;
    timer.play();

    batchDynamicUpdate(E_del, E_ins, level, parent, UP);
    rollback(E_del, E_ins);
    
    timer.pause();
    
    // Below part will not be calculate in time
    result.level = level;
    result.parent = parent;
    result.usedPrediction = false;
    result.lastMatchedStep = m_lastMatched;
    return result;
}

QueryResult FullyDynamicBFS::processUpdateNonTrivial(int step, const EdgeUpdate& realUpdate, Timer& timer)
{
    timer.play();
    realHash.insert(realUpdate);
    predHash.insert(m_predictedUpdates[step-1]);
    m_realHistory.push_back(realUpdate);
    if(realUpdate.type == UpdateType::DELETE){
        --currEdgeCnt;
    }else{
        ++currEdgeCnt;
    }

    timer.pause();

    QueryResult result;
    result.step = step;

    // Case 1: exact sequence match
    if (realHash.sameSet(predHash))
    {
        m_lastMatched = step;
        // const auto& snap = m_snapshots[m_lastMatched];
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
        if(e.type == UpdateType::DELETE){
            prevOutList[e.u].erase(e.v);
            prevInList[e.v].erase(e.u);
        }else{
            prevOutList[e.u].insert(e.v);
            prevInList[e.v].insert(e.u);
        }
    }
    prevIdx = m_lastMatched;

    int base = m_lastMatched;
    std::unordered_set<EdgeUpdate, EdgeUpdateHash> unmatchedActual;
    for (int t = base + 1; t <= step; ++t)
    {
        unmatchedActual.insert(m_realHistory[t - 1]);
    }

    std::unordered_set<EdgeUpdate, EdgeUpdateHash> unmatchedPred;

    int optimalPos = base;
    int bestCost = static_cast<int>(unmatchedActual.size()); // k = base, so P = empty

    for (int k = base + 1; k <= step; ++k)
    {
        const auto& predUpdate = m_predictedUpdates[k - 1];
        auto it = unmatchedActual.find(predUpdate);
        if (it != unmatchedActual.end())
        {
            unmatchedActual.erase(it); // matched with an actual update
        }
        else
        {
            unmatchedPred.insert(predUpdate); // extra predicted update
        }
        int curCost = static_cast<int>(unmatchedActual.size() + unmatchedPred.size());
        if (curCost < bestCost)
        {
            bestCost = curCost;
            optimalPos = k;
        }
    }

    std::unordered_set<EdgeUpdate, EdgeUpdateHash> A;
    std::unordered_set<EdgeUpdate, EdgeUpdateHash> P;

    for (int t = base + 1; t <= step; ++t)
    {
        A.insert(m_realHistory[t - 1]);
    }

    for (int t = base + 1; t <= optimalPos; ++t)
    {
        const auto& predUpdate = m_predictedUpdates[t - 1];
        auto it = A.find(predUpdate);
        if (it != A.end())
        {
            A.erase(it);   // matched update
        }
        else
        {
            P.insert(predUpdate);
        }
    }
    
    EdgeList extraEdge;
    for (int k = m_lastMatched + 1; k <= optimalPos; ++k)
    {
        const auto& e = m_predictedUpdates[k - 1];
        extraEdge.push_back(e);
        if (e.type == UpdateType::DELETE)
        {
            prevOutList[e.u].erase(e.v);
            prevInList[e.v].erase(e.u);
        }
        else
        {
            prevOutList[e.u].insert(e.v);
            prevInList[e.v].insert(e.u);
        }
    }

    EdgeList E_del, E_ins;
    for (const auto& upd : P)
    {
        if (upd.type == UpdateType::INSERT)
        {
            E_del.push_back(upd);// reverse unmatched predicted insertion
        }
        else
        {
            E_ins.push_back(upd); // reverse unmatched predicted deletion
        }
    }

    for (const auto& upd : A)
    {
        if (upd.type == UpdateType::DELETE)
        {
            E_del.push_back(upd);  // apply unmatched actual deletion directly
        }
        else
        {
            E_ins.push_back(upd); // apply unmatched actual insertion directly
        }
    }

    removeCommonEdges(E_del, E_ins);

    timer.pause();
    // std::vector<int> level  = m_snapshots[optimalPos].level;
    // std::vector<int> parent = m_snapshots[optimalPos].parent;
    // std::vector<std::unordered_set<int>> UP = m_snapshots[optimalPos].upperParents;
    const auto& snap = snapshotAt(optimalPos);
    std::vector<int>level = snap.level;
    std::vector<int>parent = snap.parent;
    std::vector<std::unordered_set<int>> UP = snap.upperParents;
    timer.play();

    batchDynamicUpdate(E_del, E_ins, level, parent, UP);
    rollback(E_del, E_ins);
    rollback(extraEdge);

    timer.pause();

    result.level = level;
    result.parent = parent;
    result.usedPrediction = false;
    result.lastMatchedStep = optimalPos;
    return result;
}

void FullyDynamicBFS::batchDynamicUpdate(const EdgeList& dels, const EdgeList& ins, 
        vector<int>&level, vector<int>&parent, std::vector<std::unordered_set<int>>& UP)
{
    int n = m_n;
    std::vector<std::vector<int>> LR(n + 2);
    std::vector<std::unordered_set<int>> LL(n + 2); 
    int lStar = n + 1;

    for (const auto& e : dels)
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

    for (const auto& e : ins)
    {
        prevOutList[e.u].insert(e.v);
        prevInList[e.v].insert(e.u);

        if (level[e.u] == INF_LEVEL) continue;

        if (level[e.v] > level[e.u] + 1)
        {
            parent[e.v] = e.u;
            level[e.v] = level[e.u] + 1;
            UP[e.v] = {e.u};
            LR[level[e.v]].push_back(e.v);
            lStar = std::min(lStar, level[e.v]);
        }
        else if (level[e.v] == level[e.u] + 1)
        {
            UP[e.v].insert(e.u);
        }
    }

    int cnt = ins.size() + dels.size();

    for (int l = lStar; l <= n; ++l)
    {
        if(cnt > (currEdgeCnt/2)){
            cnt = -1;
            break;
        }
        repairLevel(LL, l, level, parent, UP, n, prevInList, prevOutList, cnt);
        if(cnt > (currEdgeCnt/2)){
            cnt = -1;
            break;
        }

        for (auto y: LR[l])
        {
            cnt += prevOutList[y].size();
            for (int z : prevOutList[y])
            {
                if (LL[l + 1].count(z))
                {
                    LL[l + 1].erase(z);
                    parent[z] = y;
                    level[z] = l + 1;
                    UP[z].insert(y);
                    continue;
                }
                if (level[z] > level[y] + 1)
                {
                    level[z] = level[y] + 1;
                    parent[z] = y;
                    UP[z] = {y};
                    LR[level[z]].push_back(z);
                }
                else if (level[z] == level[y] + 1)
                {
                    UP[z].insert(y);
                }
            }
        }

        LL[l].clear();
        LR[l].clear();
    }

    if(cnt == -1){
        fallback_BFS(m_source, m_n, level, parent, prevOutList);
        // ++fall;
    }
}




