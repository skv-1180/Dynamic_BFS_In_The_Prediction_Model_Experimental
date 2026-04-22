#pragma once
// ============================================================
// SpaceOptimized.h
// Space-efficient incremental BFS with predictions (§6).
//
// Replaces O(mn) full snapshots with per-vertex change lists:
//   changeList[v] = { (step_t, level_t, parent_t), ... }
// Total entries ≤ n(n-1) → O(n²) space.
// Access: O(log n) per vertex via binary search.
//
// Case-2 still runs batchInsertEdge from G_i reconstructed
// via G_0 replay — same correctness as §3 Incremental.
// The space saving is only in the snapshot storage, not in
// the per-update work (Case 2 is still O(η_e + η_v)).
// ============================================================
#include <vector>
#include <tuple>
#include "Types.h"
#include "BFSState.h"

struct ChangeEntry {
    int step;   // predicted step at which the change occurred
    int level;  // new level
    int parent; // new parent
};

using ChangeList = std::vector<ChangeEntry>;

class SpaceOptimizedIncremental {
public:
    SpaceOptimizedIncremental(int numVertices, int source,
                              const EdgeList& initialEdges,
                              const EdgeList& predictedUpdates);

    // Reconstruct level[v] at predicted step j.  O(log n).
    int getLevelAt (int v, int j) const;
    int getParentAt(int v, int j) const;

    // Reconstruct full snapshot at predicted step j.  O(n log n).
    BFSSnapshot getSnapshotAt(int j) const;

    QueryResult processUpdate(int step, const EdgeUpdate& realUpdate);

    int             lastMatchedStep() const { return m_lastMatched; }
    const BFSState& realGraph()       const { return m_realGraph; }
    int             numVertices()     const { return m_n; }
    int             source()          const { return m_source; }

private:
    int m_n{};
    int m_source{};
    int m_lastMatched{0};
    int m_numPredicted{0};

    std::vector<int>    m_initLevel;
    std::vector<int>    m_initParent;
    std::vector<ChangeList> m_changes;

    EdgeList m_predictedUpdates;

    BFSState m_initGraph;  // G_0 adjacency — permanent, for G_i reconstruction
    BFSState m_realGraph;  // G_j adjacency — updated each step, NO BFS data
    EdgeList m_realHistory;
};
