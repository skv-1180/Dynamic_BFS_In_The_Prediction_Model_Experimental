#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include <utility>
#include "../include/Graph.h"
#include "../include/Preprocess.h"

EdgeList calculateBFSTree (int noOfVertices, const std::vector<std::set<int>>& adjGraph) {
    int source = 1;
    std::queue<int> q;
    q.push(source);

    std::vector <bool> visited(noOfVertices+1);
    visited[source] = true;

    EdgeList BFSTreeEdges {};

    while (!q.empty()) {
        int parent = q.front();
        q.pop();

        for (auto child : adjGraph[parent]) {
            if (visited[child]) continue;

            BFSTreeEdges.push_back({parent, child});
            visited[child] = true;
            q.push(child);
        }
    }

    return BFSTreeEdges;
}

std::vector<EdgeList> preprocessPredictedEdges(const Graph& graph){
    int numOfVertices = graph.m_numOfVertices;

    std::vector<std::set<int>> adjGraph (numOfVertices + 1);

    std::vector<EdgeList> preProcessedBFSTreeEdges;

    for (const auto& [u, v, type]: graph.m_initialEdges){
        adjGraph[u].insert(v);
        adjGraph[v].insert(u);
    }
    
    for (const auto& [u, v, type]: graph.m_predictedEdges) {
        if (!type){ // insertion
            adjGraph[u].insert(v);
            adjGraph[v].insert(u);
        }else{
            adjGraph[u].erase(v);
            adjGraph[v].erase(u);
        }

        preProcessedBFSTreeEdges.push_back(
            calculateBFSTree (numOfVertices, adjGraph)
        );
    }

    return preProcessedBFSTreeEdges;
}