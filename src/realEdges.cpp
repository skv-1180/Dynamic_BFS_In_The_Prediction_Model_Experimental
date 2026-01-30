#include <iostream>
#include "../include/Graph.h"
#include "../include/RealEdges.h"

EdgeList getRealEdges(
    int noOfVertices, 
    int noOfEdges, 
    int noOfAddionalEdges, 
    const EdgeList& initialEdges,
    const EdgeList& predictedEdges
) {
    std::cout << "Enter real edges (u, v, type(0/1))" << std::endl;

    EdgeList realEdges(noOfAddionalEdges);
    for (int i = 0; i < noOfAddionalEdges; ++i){
        std::cout << "Enter real edge " << i + 1 << ": ";
        int u, v;
        bool type;

        std::cin >> u >> v >> type;
        realEdges[i] = {u, v, type};    
    }

    return realEdges;
}
