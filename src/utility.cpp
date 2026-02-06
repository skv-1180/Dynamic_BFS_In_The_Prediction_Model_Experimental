#include <vector>
#include <queue>
#include "../include/utility.h"

void calculateBFSTree (
    int noOfVertices, 
    const std::vector<std::vector<int>>& adjGraph,
    std::vector<int>& dist,
    std::vector<int>& parent
) {
    int source = 1;
    std::queue<int> q;
    q.push(source);

    std::vector <bool> visited(noOfVertices+1);
    visited[source] = true;

    dist.assign(noOfVertices+1, -1);
    dist[source] = 0;
    parent[source] = -1;

    while (!q.empty()) {
        int par = q.front();
        q.pop();

        for (auto child : adjGraph[par]) {
            if (visited[child]) continue;
            parent[child] = par;
            dist[child] = dist[par] + 1;
            visited[child] = true;
            q.push(child);
        }
    }
}
