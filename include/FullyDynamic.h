#pragma once
#include <vector>

#include "BFSState.h"
#include "Config.h"
#include "Types.h"

class FullyDynamicBFS {
   public:
    FullyDynamicBFS(int numVertices, int source, 
        const EdgeList& initialEdges, const EdgeList& predictedUpdates);

    int lastMatchedStep() const;
    const BFSState& realGraph() const;
    int numVertices() const;
    int source() const;
    QueryResult processUpdate(int step, const EdgeUpdate& realUpdate);

   private:
    int m_n;
    int m_source;
    int m_lastMatched{0};

    EdgeList m_predictedUpdates;
    std::vector<BFSSnapshot> m_snapshots;
    BFSState m_realGraph;
    EdgeList m_realHistory;

    QueryResult fallbackBFS(int step);
};

void batchDynamicUpdate(BFSState& state, const EdgeList& deletes, const EdgeList& inserts);
