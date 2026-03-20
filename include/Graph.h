#pragma once

// ============================================================
// Graph.h  –  problem-instance container
// ============================================================

#include <iostream>
#include "Types.h"
#include "Config.h"

class Graph {
public:
    Graph() = default;
    Graph(int numVertices, int source,
          EdgeList initialEdges,
          EdgeList predictedUpdates,
          EdgeList realUpdates);

    int             numVertices()        const { return m_numVertices; }
    int             source()             const { return m_source; }
    const EdgeList& initialEdges()       const { return m_initialEdges; }
    const EdgeList& predictedUpdates()   const { return m_predictedUpdates; }
    const EdgeList& realUpdates()        const { return m_realUpdates; }
    int             numUpdates()         const { return (int)m_realUpdates.size(); }

    void print(std::ostream& os = std::cout) const;

private:
    int      m_numVertices{};
    int      m_source{DEFAULT_SOURCE};
    EdgeList m_initialEdges;
    EdgeList m_predictedUpdates;
    EdgeList m_realUpdates;
};
