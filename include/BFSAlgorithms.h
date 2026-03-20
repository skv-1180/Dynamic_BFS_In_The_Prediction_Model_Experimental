#pragma once

// ============================================================
// BFSAlgorithms.h
// Classical single-edge ES-tree updates and the three batch
// algorithms from the paper:
//   batchInsertEdge    – Theorem 1 (incremental)
//   batchDeleteEdge    – Theorem 2 (decremental)
//   batchDynamicUpdate – Theorem 3 (fully dynamic)
//
// All batch routines operate on a *temporary* BFSState that the
// caller constructs and discards.  The real-graph adjacency is
// never modified here.
// ============================================================

#include <vector>
#include "BFSState.h"
#include "Types.h"

// -------------------------------------------------------
// Classical single-edge updates (ES-tree)
// These mutate `state` in-place and maintain level/parent/UP.
// Used by the preprocessing phase.
// -------------------------------------------------------

// Incremental: insert directed edge (u -> v).
void classicalInsertEdge(BFSState& state, int u, int v);

// Decremental: delete directed edge (u -> v).
void classicalDeleteEdge(BFSState& state, int u, int v);

// -------------------------------------------------------
// Batch algorithms (paper algorithms)
// All operate on a WorkingState that is already initialised
// with the BFS data from snapshot[i] and the adjacency of
// G_i (reconstructed by buildWorkingState()).
// -------------------------------------------------------

// Section 3 – batchInsertEdge.
// Processes a batch of edge insertions {e_{i+1},...,e_j}.
// `state.outAdj` must already contain those edges (they were
// added by buildWorkingState → the adjacency is G_j).
// Updates state.level and state.parent in O(η_e + η_v).
void batchInsertEdge(BFSState& state, const EdgeList& batch);

// Section 4 – batchDeleteEdge.
// Processes a batch of edge deletions.
// `state.outAdj / inAdj` starts as G_i; this function removes
// the batch edges and repairs the BFS tree.
// Requires state.UP to be loaded from the snapshot.
// Updates state.level, state.parent, state.UP in O(η_e + η*_v).
void batchDeleteEdge(BFSState& state, const EdgeList& batch);

// Section 5 – batchDynamicUpdate.
// Processes mixed insertions and deletions.
// Requires state.UP to be loaded.
// O(min(m, η_e + η*_v)).
void batchDynamicUpdate(BFSState& state,
                        const EdgeList& deletes,
                        const EdgeList& inserts);

// -------------------------------------------------------
// Utility: build a temporary WorkingState from snapshot[i]
// and the real graph at step j.
//
// The caller provides:
//   snap          – BFSSnapshot for step i (level, parent, UP)
//   realState     – BFSState carrying G_j's adjacency
//   batchUpdates  – updates e_{i+1},...,e_j in order
//
// Returns a BFSState whose adjacency = G_i (reconstructed
// by undoing the batch updates in reverse on top of G_j)
// and whose level/parent/UP come from snap.
// -------------------------------------------------------
BFSState buildWorkingState(const BFSSnapshot& snap,
                           const BFSState&    realState,
                           const EdgeList&    batchUpdates);
