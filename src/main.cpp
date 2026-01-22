#include <iostream>
#include <vector>
#include "../include/Graph.h"
#include "../include/InitGraph.h"

int main() {
    std::cout << "Enter number of vertices, initialEdges and addionalEdges of graph: ";

    int noOfVertices, noOfInitialEdges, noOfAddionalEdges;
    std::cin >> noOfVertices >> noOfInitialEdges >> noOfAddionalEdges;

    Graph graph = initGraph(noOfVertices, noOfInitialEdges, noOfAddionalEdges);

    graph.printGraphMembers();

    return 0;
}
