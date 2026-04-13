#pragma once
#include <iostream>

#include "Config.h"
#include "Types.h"

class Graph {
public:
    Graph() = default;
    Graph(int numVertices, int source,
          EdgeList initialEdges,    
          EdgeList predictedUpdates,
          EdgeList realUpdates);

    int numVertices() const;
    int source() const;
    const EdgeList& initialEdges() const;
    const EdgeList& predictedUpdates() const;
    const EdgeList& realUpdates() const;
    int numUpdates() const;
    void print(std::ostream& os = std::cout) const;

private:
    int m_numVertices{};
    int m_source{DEFAULT_SOURCE};
    EdgeList m_initialEdges;
    EdgeList m_predictedUpdates;
    EdgeList m_realUpdates;
};
