#pragma once

// ============================================================
// Decremental.h
// Online decremental BFS with predictions (Section 4).
//
// Preprocessing: O(m²) time and space (stores UP sets).
// Per-update:    O(min(m, η_e + η*_v)) worst case (Case 2).
// ============================================================

#include <vector>
#include "Types.h"
#include "BFSState.h"

class DecrementalBFS {
public:
    DecrementalBFS(int numVertices,
                   int source,
                   const EdgeList& initialEdges,
                   const EdgeList& predictedUpdates);

    QueryResult processUpdate(int step, const EdgeUpdate& realUpdate);

    int lastMatchedStep() const { return m_lastMatched; }
    const BFSState& realGraph() const { return m_realGraph; }
    int numVertices() const { return m_n; }
    int source()      const { return m_source; }

private:
    int m_n;
    int m_source;
    int m_lastMatched{0};

    EdgeList                  m_predictedUpdates;
    std::vector<BFSSnapshot>  m_snapshots;
    BFSState                  m_realGraph;
    EdgeList                  m_realHistory;
};
