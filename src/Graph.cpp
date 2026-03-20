#include "../include/Graph.h"

Graph::Graph(int numVertices, int source,
             EdgeList initialEdges,
             EdgeList predictedUpdates,
             EdgeList realUpdates)
    : m_numVertices(numVertices)
    , m_source(source)
    , m_initialEdges(std::move(initialEdges))
    , m_predictedUpdates(std::move(predictedUpdates))
    , m_realUpdates(std::move(realUpdates))
{}

void Graph::print(std::ostream& os) const
{
    os << "Vertices: " << m_numVertices
       << "  Source: " << m_source << "\n";

    os << "Initial edges (" << m_initialEdges.size() << "):\n";
    for (const auto& e : m_initialEdges)
        os << "  " << e.u << " -> " << e.v << "\n";

    os << "Predicted updates (" << m_predictedUpdates.size() << "):\n";
    for (int i = 0; i < (int)m_predictedUpdates.size(); ++i) {
        const auto& e = m_predictedUpdates[i];
        os << "  [" << i+1 << "] "
           << (e.type == UpdateType::INSERT ? "+" : "-")
           << "(" << e.u << "," << e.v << ")\n";
    }

    os << "Real updates (" << m_realUpdates.size() << "):\n";
    for (int i = 0; i < (int)m_realUpdates.size(); ++i) {
        const auto& e = m_realUpdates[i];
        os << "  [" << i+1 << "] "
           << (e.type == UpdateType::INSERT ? "+" : "-")
           << "(" << e.u << "," << e.v << ")\n";
    }
}
