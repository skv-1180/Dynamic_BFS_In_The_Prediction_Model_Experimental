#pragma once
// ============================================================
// SpaceOptimizedDecremental.h
// Space-efficient decremental BFS with predictions (paper §new).
//
// Instead of storing full UP[v] sets for every predicted snapshot
// (O(m²) space), we store one POINTER per vertex per snapshot:
//
//   ptr[i][v] = index into In_i[v] (position of scan cursor)
//
// CURSOR INVARIANT (paper §new):
//   ptr[v] points to the first unexamined incoming neighbour in
//   In[v]. When v's parent is invalidated, advance the cursor
//   until a neighbour at level[v]−1 is found. If found → new
//   parent. If cursor exhausted → level[v] must increase; reset
//   ptr[v] to start of In[v] at the new level.
//
// SPACE BOUND:
//   Each checkpoint stores one int per vertex → O(n) per step.
//   With checkpoint interval k=1 (every step): O(mn) total.
//   With checkpoint interval k: O(mn/k) total space.
//   Recovery from checkpoint t ≤ i: replay at most k−1 steps.
//
// CHECKPOINT INTERVAL k:
//   k=1  (default)  – store every predicted step, O(mn) space.
//   k>1             – store every k-th step, O(mn/k) space,
//                     O(k) extra work per Case-2 query.
// ============================================================
#include <vector>
#include "Types.h"
#include "BFSState.h"
#include "Config.h"

// ── Per-vertex pointer entry (one per stored checkpoint) ─────
struct PtrEntry {
    int step{0};     // predicted step this checkpoint belongs to
    int ptr{0};      // cursor index into In[v] at this step
    int level{INF_LEVEL};
    int parent{NO_PARENT};
};

// One checkpoint: level[], parent[], and ptr[] for all vertices.
struct DecrementalCheckpoint {
    int              step{0};
    std::vector<int> level;    // level[1..n]
    std::vector<int> parent;   // parent[1..n]
    std::vector<int> ptr;      // cursor index into inAdj[v], 0-indexed
    // inAdj is NOT stored here — it is reconstructed from G_0 + realHistory.
};

class SpaceOptimizedDecremental {
public:
    // k = checkpoint interval (default 1 = store every step = O(mn) space).
    SpaceOptimizedDecremental(int numVertices,
                              int source,
                              const EdgeList& initialEdges,
                              const EdgeList& predictedUpdates,
                              int k = 1);

    // Process real update at step j.
    QueryResult processUpdate(int step, const EdgeUpdate& realUpdate);

    // Reconstruct BFSSnapshot at predicted step i from checkpoints.
    // O(n) when k=1; O(n·k) for general k (replay up to k-1 deletions).
    BFSSnapshot getSnapshotAt(int i) const;

    int             lastMatchedStep() const { return m_lastMatched; }
    const BFSState& realGraph()       const { return m_realGraph; }
    int             numVertices()     const { return m_n; }
    int             source()          const { return m_source; }
    int             checkpointK()     const { return m_k; }

private:
    // Replay deletion from a checkpoint up to target step.
    // Returns the BFSSnapshot at targetStep.
    BFSSnapshot replayFromCheckpoint(const DecrementalCheckpoint& cp,
                                     int targetStep) const;

    int m_n{};
    int m_source{};
    int m_k{1};           // checkpoint interval
    int m_lastMatched{0};
    int m_numPredicted{0};

    EdgeList m_predictedUpdates;

    // Checkpoints: stored at steps 0, k, 2k, ...
    std::vector<DecrementalCheckpoint> m_checkpoints;

    // G_0 adjacency (permanent — needed to replay deletions).
    BFSState m_initGraph;

    // G_j adjacency (updated every real step).
    BFSState m_realGraph;

    // G_0 edge list as a sorted vector per vertex (for cursor replay).
    // inAdj_sorted[v] = sorted list of in-neighbours of v in G_0.
    std::vector<std::vector<int>> m_inAdj0_sorted;

    EdgeList m_realHistory;
};
