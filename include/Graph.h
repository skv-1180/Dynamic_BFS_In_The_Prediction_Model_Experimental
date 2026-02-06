#pragma once
#include <iostream>
#include <vector>

struct Edge
{
    int u{};
    int v{};
    bool type{}; //0 for insertion and 1 for deletion
};

using EdgeList = std::vector<Edge>;

struct BFSEntry {
    int v;
    int parent;
    int dist;
};

using BFSEntryList = std::vector<BFSEntry>;

class Graph{ // Source is assumed to be 1
private:
    int m_numOfVertices{};
    int m_numOfInitialEdges{};
    int m_noOfAddionalEdges{};
    EdgeList m_initialEdges{};
    EdgeList m_predictedEdges{};
    EdgeList m_realEdges{};

    std::vector<int> m_initialParent;
    std::vector<int> m_initialDist;
    std::vector<BFSEntryList> m_changeLists{};
public:
    Graph(
        int numOfVertices,
        int numOfInitialEdges,
        int noOfAddionalEdges,
        EdgeList initialEdges,
        EdgeList predictedEdges,
        EdgeList realEdges
    );

    int getNumOfVertices() const;
    int getNumOfEdges() const;
    int getNumOfAdditionalEdges() const;

    const EdgeList& getInitialEdges() const;
    const EdgeList& getPredictedEdges() const;
    const EdgeList& getRealEdges() const;
    
    const std::vector<EdgeList>& getPreprocessedBFSTreeEdges() const;

    void setInitialDistance(const std::vector<int>& initialDist);
    void setInitialParent(const std::vector<int>& initialParent);
    void setChangeLists(std::vector<BFSEntryList> changeLists);

    void printGraphMembers() const;

};
