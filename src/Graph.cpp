#include "../include/Graph.h"
#include <utility>

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

const EdgeList& Graph::getInitialEdges() const {
    return m_initialEdges;
}

const EdgeList& Graph::getPredictedEdges() const {
    return m_predictedEdges;
}

const EdgeList& Graph::getRealEdges() const {
    return m_realEdges;
}

int Graph::getNumOfAdditionalEdges() const {
    return m_noOfAddionalEdges;
}

void Graph::setInitialDistance(const std::vector<int>& initialDist){
    m_initialDist = initialDist;
}

void Graph::setInitialParent(const std::vector<int>& initialParent){
    m_initialParent = initialParent;
}

void Graph::setChangeLists(std::vector<BFSEntryList> changeLists){
    m_changeLists = std::move(changeLists);
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

    std::cout << "==== Initial parent and distance (node, parent, distance) ==== " << std::endl;
    for(int u = 1; u <= m_numOfVertices; ++u){
        std::cout << u << ' ' << m_initialParent[u] << ' ' << m_initialDist[u] << std::endl;
    }

    std::cout << "==== Change Lists (node, parent, distance) ====" << std::endl;

    for (int i = 0; i < m_noOfAddionalEdges; ++i) {
        const auto& predictedEdge = m_predictedEdges[i];

        std::cout << "After processing predicted edge: ("
                  << predictedEdge.u << ' ' << predictedEdge.v << ' '
                  << predictedEdge.type << ") Changes in BFS Tree are: " << std::endl;

        for (const auto& changes : m_changeLists[i]) {
            std::cout << changes.v << ' ' << changes.parent << ' ' 
            << changes.dist << std::endl;
        }

        std::cout << std::endl;
    }
}
