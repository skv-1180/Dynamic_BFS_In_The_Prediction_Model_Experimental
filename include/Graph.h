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
    )
    :   m_numOfVertices{numOfVertices}, 
        m_numOfInitialEdges{numOfInitialEdges},
        m_noOfAddionalEdges{noOfAddionalEdges},
        m_initialEdges{initialEdges},
        m_predictedEdges{predictedEdges},
        m_realEdges{realEdges}
    {
    }

    int getNumOfVertices() const {
        return m_numOfVertices;
    }

    int getNumOfEdges() const {
        return m_numOfInitialEdges;
    }

    EdgeList getInitialEdges() const {
        return m_initialEdges;
    }

    void setPreprocessedBFSTreeEdges(const std::vector<EdgeList>& preprocessedBFSTreeEdges) {
        m_preProcessedBFSTreeEdges = preprocessedBFSTreeEdges;
    }

    void printGraphMembers() const;

    friend std::vector<EdgeList> preprocessPredictedEdges(const Graph& graph);


};
