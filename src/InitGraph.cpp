#include "../include/Graph.h"
#include "../include/InitGraph.h"

Graph initGraph() {
    int noOfVertices, noOfInitialEdges, noOfAddionalEdges;
    std::cin >> noOfVertices >> noOfInitialEdges >> noOfAddionalEdges;

    EdgeList initialEdges(noOfInitialEdges);
    for (int i = 0; i < noOfInitialEdges; ++i){
        int u, v;
        std::cin >> u >> v;
        initialEdges[i] = {u, v, 0};
    }

    EdgeList predictedEdges(noOfAddionalEdges);
    for (int i = 0; i < noOfAddionalEdges; ++i){
        int u, v;
        bool type;

        std::cin >> u >> v >> type;
        predictedEdges[i] = {u, v, type};    
    }

    EdgeList realEdges(noOfAddionalEdges);
    for (int i = 0; i < noOfAddionalEdges; ++i){
        int u, v;
        bool type;

        std::cin >> u >> v >> type;
        realEdges[i] = {u, v, type};    
    }

    Graph graph{noOfVertices, noOfInitialEdges, noOfAddionalEdges, 
        initialEdges, predictedEdges, realEdges};
    
    return graph;
}