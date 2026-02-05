#include <iostream>
#include <vector>
#include "../include/Graph.h"
#include "../include/InitGraph.h"
#include "../include/Preprocess.h"
#include "../include/BFSSnapshotStore.h"

int main() {
    Graph graph = initGraph();

    // Build per-update change-lists using Method 2 and reconstruct full trees for compatibility.
    BFSSnapshotStore store;
    store.buildFromGraph(graph);

    // Reconstruct all full BFS trees (one per predicted update) and set them into Graph for printing
    std::vector<EdgeList> allTrees = store.reconstructAllFullTrees();
    graph.setPreprocessedBFSTreeEdges(allTrees);

    graph.printGraphMembers();

    // Example usage:
    // if you want to retrieve the t-th tree on demand:
    // int idx = 0; // 0-based index of predicted update
    // EdgeList treeAtIdx = store.getBFSTreeAtIndex(idx);
    //
    // or find the index for a given predicted Edge:
    // Edge pe = graph.getPredictedEdges()[0];
    // int index = store.indexOfPredictedEdge(pe);
    //
    return 0;
}
