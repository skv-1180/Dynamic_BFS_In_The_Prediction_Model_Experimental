#include <iostream>
#include "../include/Graph.h"
#include "../include/PredictedEdges.h"

EdgeList getPredictedEdges(
    int noOfVertices, 
    int noOfEdges, 
    int noOfAddionalEdges, 
    const EdgeList& initialEdges
) {
    std::cout << "Enter predicted edges (u, v, type(0/1))" << std::endl;

    EdgeList predictedEdges(noOfAddionalEdges);
    for (int i = 0; i < noOfAddionalEdges; ++i){
        std::cout << "Enter predicted edge " << i + 1 << ": ";
        int u, v;
        bool type;

        std::cin >> u >> v >> type;
        predictedEdges[i] = {u, v, type};    
    }

    return predictedEdges;
}