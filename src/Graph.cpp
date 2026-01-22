#include "../include/Graph.h"

void Graph::printGraphMembers() const {
    std::cout << "No of vertices: " << m_numOfVertices << std::endl; 
    std::cout << "No of edges: " << m_numOfVertices << std::endl; 
    std::cout << "No of vertices: " << m_numOfVertices << std::endl; 

    std::cout << "==== Initial Edges ====" << std::endl;
    for (const auto& edge: m_initialEdges) {
        std::cout << edge.u << ' ' << edge.v << std::endl;
    }
    
    std::cout << "==== Predicted Edges ====" << std::endl;
    for (const auto& edge: m_predictedEdges) {
        std::cout << edge.u << ' ' << edge.v << ' ' << edge.type << std::endl;
    }
    
    std::cout << "==== Real Edges ====" << std::endl;
    for (const auto& edge: m_realEdges) {
        std::cout << edge.u << ' ' << edge.v << ' ' << edge.type << std::endl;
    }
}