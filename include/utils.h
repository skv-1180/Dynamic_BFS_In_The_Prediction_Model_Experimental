#pragma once
#include <vector>
#include "BFSState.h"
#include "Types.h"
#include <unordered_map>
using namespace std;

void buildPredEdgeIdx(unordered_map<EdgeUpdate, int, EdgeUpdateHash>& predEdgeIdx, const EdgeList& predictedUpdates);

// for decremental currently
void repairLevel(
    std::vector<std::unordered_set<int>>& LL, 
    int l, 
    vector<int>&level, 
    vector<int>&parent, 
    std::vector<std::unordered_set<int>>& UP,
    int n,
    const std::vector<std::unordered_set<int>>& prevInList,
    const std::vector<std::unordered_set<int>>& prevOutList
);


void repairLevel(BFSState& ws, std::vector<std::set<int>>& LL, int l);

void removeCommonEdges(EdgeList& E_del, EdgeList& E_ins);

BFSState buildWorkingState(const BFSSnapshot& snap,
                           const BFSState&    realState,
                           const EdgeList&    batchUpdates);

