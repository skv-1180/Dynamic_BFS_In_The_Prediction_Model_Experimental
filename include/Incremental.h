#pragma once
#include <bits/stdc++.h>

#include <unordered_map>
#include <vector>

#include "BFSState.h"
#include "Types.h"
#include "timer.h"
using namespace std;

#define TrivialError true

class IncrementalBFS
{
   public:
    IncrementalBFS(int numVertices, int source,
                   const EdgeList& initialEdges,
                   const EdgeList& predictedUpdates, 
                   ErrorCorrectionMode errorCorrMode = {ErrorCorrectionMode::TRIVIAL});


    QueryResult processUpdate(int step, const EdgeUpdate& realUpdate, Timer& timer);
    
    const BFSSnapshot& snapshotAt(int i) const { return m_snapshots[i]; }
    int numSnapshots() const { return (int)m_snapshots.size(); }
    int lastMatchedStep() const { return m_lastMatched; }
    int numVertices() const { return m_n; }
    int source() const { return m_source; }

private:
    void initGraphs(const EdgeList& initialEdges);
    void batchInsertEdge(const EdgeList& batch, vector<int>&level, vector<int>&parent);
    void rollback(const EdgeList& batch);
    QueryResult processUpdateTrivial(int step, const EdgeUpdate& realUpdate, Timer& timer);
    QueryResult processUpdateNonTrivial(int step, const EdgeUpdate& realUpdate, Timer& timer);

private:
    int m_n{};
    int m_source{};
    int m_lastMatched{0};

    EdgeList m_predictedUpdates;
    std::vector<BFSSnapshot> m_snapshots;  // 1-indexed: idx 0 stores G_0 snapshot, idx i stores snapshot after predicted update i(in code i-1 by index)
    EdgeList m_realHistory;
    ErrorCorrectionMode m_errorCorrMode{ErrorCorrectionMode::TRIVIAL};
    unordered_map<EdgeUpdate, int, EdgeUpdateHash> predEdgeIdx;
    int maxUpdateIdx{0};
    int prevIdx{0};  // lastIdxUptoWhichRealUpdatesAreInsertedInRunningGraph
    vector<unordered_set<int>> prevOutList; 
};

