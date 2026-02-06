#include <iostream>
#include <vector>
#include <queue>
#include <cassert>
#include "../include/Graph.h"
#include "../include/utility.h"
#include "../include/incremental.h"

void processRealEdges(Graph& graph){
    int numOfVertices = graph.getNumOfVertices();
    std::vector<std::vector<int>> adjGraph (numOfVertices + 1);

    for (const auto& [u, v, type]: graph.getInitialEdges()){
        assert(type == 0 && "type should be zero in incremental case");
        adjGraph[u].push_back(v);
        adjGraph[v].push_back(u);
    }

    std::vector<int> distance = graph.getInitialDistance();
    std::vector<int> parent = graph.getInitialParent();

    int noOfAddionalEdges = graph.getNumOfAdditionalEdges();
    const auto& RealEdges = graph.getRealEdges();
    const auto& PredictedEdges = graph.getPredictedEdges();

    bool isPrefixEqual = true;
    for (int i = 0; i < noOfAddionalEdges; ++i){
        Edge realEdge = RealEdges[i];
        Edge predictedEdge = PredictedEdges[i];
        assert(realEdge.type == 0 && "type should be zero in incremental case");
        assert(predictedEdge.type == 0 && "type should be zero in incremental case");

        if(!(realEdge == predictedEdge)) {
            isPrefixEqual = false;
        } 
        // -------
    }   
}

BFSEntryList insertEdge(
    const Edge& newEdge,
    std::vector<std::vector<int>>& adjGraph,
    std::vector<int>& dist,
    std::vector<int>& parent
)
{
    int u = newEdge.u;
    int v = newEdge.v;

    adjGraph[u].push_back(v);
    adjGraph[v].push_back(u);

    BFSEntryList changes;

    std::queue<int> q;

    auto tryRelax = [&](int a, int b)
    {
        if (dist[a] == -1) return;

        if ((dist[b] == -1) || (dist[b] > dist[a] + 1))
        {
            dist[b] = dist[a] + 1;
            parent[b] = a;

            changes.push_back({b, a, dist[b]});
            q.push(b);
        }
    };

    tryRelax(u, v);
    tryRelax(v, u);

    while (!q.empty())
    {
        int par = q.front();
        q.pop();

        for (int child : adjGraph[par])
        {
            if (dist[child] == -1 || dist[child] > dist[par] + 1)
            {
                dist[child] = dist[par] + 1;
                parent[child] = par;

                changes.push_back({child, par, dist[child]});
                q.push(child);
            }
        }
    }

    return changes;
}

void preprocessPredictedEdges(Graph& graph){
    int numOfVertices = graph.getNumOfVertices();

    std::vector<std::vector<int>> adjGraph (numOfVertices + 1);

    for (const auto& [u, v, type]: graph.getInitialEdges()){
        assert(type == 0 && "type should be zero in incremental case");
        adjGraph[u].push_back(v);
        adjGraph[v].push_back(u);
    }
    
    std::vector<int> distance (numOfVertices + 1, -1);
    std::vector<int> parent (numOfVertices + 1, -1);
    calculateBFSTree(numOfVertices, adjGraph, distance, parent);

    graph.setInitialDistance(distance);
    graph.setInitialParent(parent);

    std::vector<BFSEntryList> changeLists {};
    for (const auto& edge: graph.getPredictedEdges()) {
        assert(edge.type == 0 && "type should be zero in incremental case");
        BFSEntryList changes = insertEdge(edge, adjGraph, distance, parent);
        changeLists.push_back(std::move(changes));
    }

    graph.setChangeLists(std::move(changeLists));
}