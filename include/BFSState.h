#pragma once

// ============================================================
// BFSState.h
// A "working copy" of the full BFS state: adjacency lists
// (in and out), level array, parent array, and upper-parent
// sets.  Used both as the real-graph tracker and as the
// temporary state for the batch algorithms.
// ============================================================

#include <iostream>
#include <set>
#include <vector>

#include "Config.h"
#include "Types.h"

class BFSState {
   public:
    int n{};       
    int source{};  

    std::vector<int> level;            
    std::vector<int> parent;           
    std::vector<std::set<int>> UP;     
    std::vector<std::set<int>> outAdj; 
    std::vector<std::set<int>> inAdj;  

    BFSState(int n, int source);
    BFSState(const BFSState&) = default;
    BFSState& operator=(const BFSState&) = default;

    void addEdge(int u, int v);     
    void removeEdge(int u, int v);  
 
    // Recompute level and parent from scratch
    void computeBFS();
    void computeAllUP();
    BFSSnapshot saveSnapshot(bool saveUP = false) const;
    void loadSnapshot(const BFSSnapshot& snap);
    void printBFSTree(std::ostream& os = std::cout) const;
    void printAdjacency(std::ostream& os = std::cout) const;
    bool verify() const;
};
