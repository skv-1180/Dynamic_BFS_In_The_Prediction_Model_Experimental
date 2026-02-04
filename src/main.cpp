#include <iostream>
#include <vector>
#include "../include/Graph.h"
#include "../include/InitGraph.h"
#include "../include/Preprocess.h"

int main() {
    Graph graph = initGraph();

    graph.setPreprocessedBFSTreeEdges(preprocessPredictedEdges(graph));

    graph.printGraphMembers();
    
    return 0;
}
