#pragma once
#include <bits/stdc++.h>
#include <unordered_map>
#include <vector>

#include "BFSState.h"
#include "Types.h"
#include "timer.h"
using namespace std;

class DecrementalBFS
{
   public:
    DecrementalBFS(int numVertices, int source,
                   const EdgeList& initialEdges,
                   const EdgeList& predictedUpdates,
                   ErrorCorrectionMode errorCorrMode = {ErrorCorrectionMode::TRIVIAL});

    QueryResult processUpdate(int step, const EdgeUpdate& realUpdate, Timer& timer);

    BFSSnapshot snapshotAt(int i) const;
    int numSnapshots() const { return (int)m_snapshots.size(); }
    int lastMatchedStep() const { return m_lastMatched; }
    int numVertices() const { return m_n; }
    int source() const { return m_source; }

private:
    void initGraphs(const EdgeList& initialEdges);
    void batchDeleteEdge(const EdgeList& batch, vector<int>&level, vector<int>&parent, std::vector<std::unordered_set<int>>& UP);
    void rollback(const EdgeList& batch);
    QueryResult processUpdateTrivial(int step, const EdgeUpdate& realUpdate, Timer& timer);
    QueryResult processUpdateNonTrivial(int step, const EdgeUpdate& realUpdate, Timer& timer);

    int m_n{};
    int m_source{};
    int m_lastMatched{0};

    EdgeList m_predictedUpdates;
    std::vector<BFSSnapshot> m_snapshots;  // includes UP sets
    EdgeList m_realHistory;
    
    ErrorCorrectionMode m_errorCorrMode{ErrorCorrectionMode::TRIVIAL};
    unordered_map<EdgeUpdate, int, EdgeUpdateHash> predEdgeIdx;
    int maxUpdateIdx{0};
    int prevIdx{0};  // lastIdxUptoWhichRealUpdatesAreInsertedInRunningGraph
    vector<unordered_set<int>> prevInList; 
    vector<unordered_set<int>> prevOutList; 
    int currEdgeCnt{0};

    EdgeList m_initialEdges; 
};
