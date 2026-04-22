// ============================================================
// InitGraph.cpp
// ============================================================
//
// Input file format
// -----------------
// Line 1:  numVertices  source  numInitialEdges  numUpdates
// Next numInitialEdges lines:  u v       (directed edge u->v in G_0)
// Next numUpdates lines:       u v type  (predicted update; type 0=insert,1=delete)
// Next numUpdates lines:       u v type  (real      update)
//
// Lines beginning with '#' are comments and are ignored.
// ============================================================

#include "../include/InitGraph.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

static void skipComments(std::istream& in)
{
    while (in.peek() == '#') {
        std::string line;
        std::getline(in, line);
    }
}

Graph readGraph(std::istream& in)
{
    int n, src, numInit, numUpdates;

    skipComments(in);
    if (!(in >> n >> src >> numInit >> numUpdates))
        throw std::runtime_error("readGraph: failed to read header");

    EdgeList initEdges(numInit);
    for (int i = 0; i < numInit; ++i) {
        int u, v;
        if (!(in >> u >> v))
            throw std::runtime_error("readGraph: failed to read initial edge " + std::to_string(i));
        initEdges[i] = {u, v, UpdateType::INSERT};
    }

    EdgeList predicted(numUpdates), real(numUpdates);

    for (int i = 0; i < numUpdates; ++i) {
        int u, v, t;
        if (!(in >> u >> v >> t))
            throw std::runtime_error("readGraph: failed to read predicted update " + std::to_string(i));
        predicted[i] = {u, v, t == 0 ? UpdateType::INSERT : UpdateType::DELETE};
    }

    for (int i = 0; i < numUpdates; ++i) {
        int u, v, t;
        if (!(in >> u >> v >> t))
            throw std::runtime_error("readGraph: failed to read real update " + std::to_string(i));
        real[i] = {u, v, t == 0 ? UpdateType::INSERT : UpdateType::DELETE};
    }

    return Graph(n, src, initEdges, predicted, real);
}

Graph readGraphFromFile(const std::string& filename)
{
    std::ifstream fin(filename);
    if (!fin.is_open())
        throw std::runtime_error("readGraphFromFile: cannot open '" + filename + "'");
    return readGraph(fin);
}
