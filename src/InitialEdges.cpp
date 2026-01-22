#include <iostream>
#include <vector>
#include "../include/Graph.h"
#include "../include/InitialEdges.h"

EdgeList getInitialEdges(int noOfVertices, int noOfEdges){
    std::cout << "Enter initial edges of graph (u, v)" << std::endl;

    EdgeList edges(noOfEdges);
    for (int i = 0; i < noOfEdges; ++i){
        std::cout << "Enter edge " << i + 1 << ": ";
        int u, v;
        std::cin >> u >> v;
        edges[i] = {u, v, 0};
    }

    return edges;
}