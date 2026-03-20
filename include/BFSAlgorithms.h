#pragma once
#include <vector>
#include "BFSState.h"
#include "Types.h"

void classicalInsertEdge(BFSState& state, int u, int v);

void classicalDeleteEdge(BFSState& state, int u, int v);

void batchInsertEdge(BFSState& state, const EdgeList& batch);
void batchDeleteEdge(BFSState& state, const EdgeList& batch);
void batchDynamicUpdate(BFSState& state, const EdgeList& deletes, const EdgeList& inserts);

BFSState buildWorkingState(const BFSSnapshot& snap,
                           const BFSState&    realState,
                           const EdgeList&    batchUpdates);
