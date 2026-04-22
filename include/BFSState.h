#pragma once

#include <iostream>
#include <set>
#include <unordered_set>
#include <vector>

#include "Config.h"
#include "Types.h"

class BFSState
{
   public:
    int n{};       
    int source{};  

    std::vector<int> level;                            
    std::vector<int> parent;                           
    std::vector<std::unordered_set<int>> UP;           
    std::vector<std::unordered_set<int>> outAdj;       
    std::vector<std::unordered_set<int>> inAdj;        
    AlgorithmMode mode{AlgorithmMode::FULLY_DYNAMIC};  
   public:
    BFSState(int n, int source, AlgorithmMode algMode = AlgorithmMode::FULLY_DYNAMIC);

    BFSState(const BFSState&) = default;
    BFSState& operator=(const BFSState&) = default;

    void addEdge(int u, int v);
    void removeEdge(int u, int v);

    void computeBFS();

    void computeAllUP();

    BFSSnapshot saveSnapshot(bool saveUP = false) const;

    void loadSnapshot(const BFSSnapshot& snap);

    void printBFSTree(std::ostream& os = std::cout) const;
    void printAdjacency(std::ostream& os = std::cout) const;

    bool verify() const;
};
