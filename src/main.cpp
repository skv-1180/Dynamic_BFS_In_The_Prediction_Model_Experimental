#include <iostream>
#include <vector>
#include "../include/Graph.h"
#include "../include/InitGraph.h"
#include "../include/incremental.h"

int main() {
    Graph graph = initGraph();
    preprocessPredictedEdges(graph);

    graph.printGraphMembers();

    
    return 0;
}
