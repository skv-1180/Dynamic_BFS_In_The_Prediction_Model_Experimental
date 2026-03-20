#pragma once

// ============================================================
// FullyDynamic.h
// Online fully-dynamic BFS with predictions (Section 5).
//
// Preprocessing: O(m²) time and space.
// Per-update:    O(min(m, η_e + η*_v)) worst case.
//
// When the batch cost exceeds O(m), falls back to full BFS
// recomputation in O(m) (the robust fallback).
// ============================================================

#include <vector>
#include "Types.h"
#include "BFSState.h"
#include "Config.h"

class FullyDynamicBFS {
public:
    FullyDynamicBFS(int numVertices,
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

    // Fall back to full BFS when batch cost would exceed O(m).
    QueryResult fallbackBFS(int step);
};
