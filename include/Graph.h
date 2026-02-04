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

class Graph{ // Source is assumed to be 1
private:
    int m_numOfVertices{};
    int m_numOfInitialEdges{};
    int m_noOfAddionalEdges{};
    EdgeList m_initialEdges{};
    EdgeList m_predictedEdges{};
    EdgeList m_realEdges{};
    std::vector<EdgeList> m_preProcessedBFSTreeEdges{};
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

    EdgeList getInitialEdges() const;

    void setPreprocessedBFSTreeEdges(const std::vector<EdgeList>& preprocessedBFSTreeEdges);

    void printGraphMembers() const;

    friend std::vector<EdgeList> preprocessPredictedEdges(const Graph& graph);
};
