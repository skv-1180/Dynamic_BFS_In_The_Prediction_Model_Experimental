#include "../include/Graph.h"

Graph::Graph(
    int numOfVertices,
    int numOfInitialEdges,
    int noOfAddionalEdges,
    EdgeList initialEdges,
    EdgeList predictedEdges,
    EdgeList realEdges)
    : m_numOfVertices{numOfVertices},
      m_numOfInitialEdges{numOfInitialEdges},
      m_noOfAddionalEdges{noOfAddionalEdges},
      m_initialEdges{initialEdges},
      m_predictedEdges{predictedEdges},
      m_realEdges{realEdges} {
}

int Graph::getNumOfVertices() const {
    return m_numOfVertices;
}

int Graph::getNumOfEdges() const {
    return m_numOfInitialEdges;
}

EdgeList Graph::getInitialEdges() const {
    return m_initialEdges;
}

void Graph::setPreprocessedBFSTreeEdges(const std::vector<EdgeList>& preprocessedBFSTreeEdges) {
    m_preProcessedBFSTreeEdges = preprocessedBFSTreeEdges;
}

void Graph::printGraphMembers() const {
    std::cout << "No of vertices: " << m_numOfVertices << std::endl;
    std::cout << "No of edges: " << m_numOfInitialEdges << std::endl;
    std::cout << "No of additonal edges: " << m_noOfAddionalEdges << std::endl;

    std::cout << "==== Initial Edges ====" << std::endl;
    for (const auto& edge : m_initialEdges) {
        std::cout << edge.u << ' ' << edge.v << std::endl;
    }

    std::cout << "==== Predicted Edges ====" << std::endl;
    for (const auto& edge : m_predictedEdges) {
        std::cout << edge.u << ' ' << edge.v << ' ' << edge.type << std::endl;
    }

    std::cout << "==== Real Edges ====" << std::endl;
    for (const auto& edge : m_realEdges) {
        std::cout << edge.u << ' ' << edge.v << ' ' << edge.type << std::endl;
    }

    std::cout << "==== Preprocessed BFS Tree ====" << std::endl;

    for (int i = 0; i < m_noOfAddionalEdges; ++i) {
        const auto& currBFSTreeEdges = m_preProcessedBFSTreeEdges[i];
        const auto& predictedEdge = m_predictedEdges[i];

        std::cout << "After processing predicted edge: ("
                  << predictedEdge.u << ' ' << predictedEdge.v << ' '
                  << predictedEdge.type << ") BFS Tree is: " << std::endl;

        for (const auto& edge : currBFSTreeEdges) {
            std::cout << edge.u << ' ' << edge.v << std::endl;
        }

        std::cout << std::endl;
    }
}