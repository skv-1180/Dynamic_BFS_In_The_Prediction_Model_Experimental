#pragma once
#include <vector>
#include "BFSState.h"
#include "Types.h"

void repairLevel(BFSState& ws, std::vector<std::set<int>>& LL, int l);
BFSState buildWorkingState(const BFSSnapshot& snap,
                           const BFSState&    realState,
                           const EdgeList&    batchUpdates);

