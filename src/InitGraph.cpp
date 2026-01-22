#include "../include/Graph.h"
#include "../include/InitGraph.h"
#include "../include/InitialEdges.h"
#include "../include/PredictedEdges.h"
#include "../include/realEdges.h"

Graph initGraph(int noOfVertices, int noOfEdges, int noOfAddionalEdges) {
    EdgeList initialEdges = getInitialEdges(noOfVertices, noOfEdges);
    EdgeList predictedEdges = getPredictedEdges(
        noOfVertices, noOfEdges, noOfAddionalEdges, initialEdges);
    EdgeList realEdges = getRealEdges(
        noOfVertices, noOfEdges, noOfAddionalEdges, initialEdges, predictedEdges);

    Graph graph{noOfVertices, noOfEdges, initialEdges, predictedEdges, realEdges};

    return graph;
}