#include "../include/utils.h"

#include <algorithm>
#include <cassert>
#include <queue>
#include <set>
#include <vector>


void buildPredEdgeIdx(unordered_map<EdgeUpdate, int, EdgeUpdateHash>& predEdgeIdx, const EdgeList& predictedUpdates){
    for(int i = 0; i < (int)predictedUpdates.size(); ++i){
        predEdgeIdx[predictedUpdates[i]] = i + 1; // 1-based index
    // can use initial edge if required later
    }
}

void removeCommonEdges(EdgeList& E_del, EdgeList& E_ins) {
    std::unordered_set<EdgeUpdate, EdgeUpdateHash> delSet;

    // Store all delete edges
    for (const auto& e : E_del) {
        delSet.insert({e.u, e.v});
    }

    EdgeList new_ins;
    EdgeList new_del;

    // Process insert edges
    for (const auto& e : E_ins) {
        EdgeUpdate key{e.u, e.v};

        if (delSet.count(key)) {
            // Found in both → cancel it
            delSet.erase(key);
        } else {
            new_ins.push_back(e);
        }
    }

    // Remaining delete edges (not cancelled)
    for (const auto& e : E_del) {
        EdgeUpdate key{e.u, e.v};
        if (delSet.count(key)) {
            new_del.push_back(e);
        }
    }

    E_del = std::move(new_del);
    E_ins = std::move(new_ins);
}

BFSState buildWorkingState(const BFSSnapshot& snap,
                           const BFSState& realState,
                           const EdgeList& batchUpdates)
{
    BFSState ws(realState.n, realState.source);

    ws.outAdj = realState.outAdj;
    ws.inAdj = realState.inAdj;

    for (int k = (int)batchUpdates.size() - 1; k >= 0; --k)
    {
        const auto& e = batchUpdates[k];
        if (e.type == UpdateType::INSERT)
        {
            ws.outAdj[e.u].erase(e.v);
            ws.inAdj[e.v].erase(e.u);
        }
        else
        {
            ws.outAdj[e.u].insert(e.v);
            ws.inAdj[e.v].insert(e.u);
        }
    }

    ws.loadSnapshot(snap);
    return ws;
}


void repairLevel(
    std::vector<std::unordered_set<int>>& LL, 
    int l, 
    vector<int>&level, 
    vector<int>&parent, 
    std::vector<std::unordered_set<int>>& UP,
    int n,
    const std::vector<std::unordered_set<int>>& prevInList,
    const std::vector<std::unordered_set<int>>& prevOutList,
    int& cnt 
){
    for (int x: LL[l])
    {
        cnt += prevInList[x].size() + prevOutList[x].size();

        if (!UP[x].empty())
        {
            parent[x] = *UP[x].begin();
            continue;
        }

        int newLevel = level[x] + 1;

        if (newLevel > n) // should be n-1
        {
            level[x] = INF_LEVEL;
            parent[x] = NO_PARENT;
            UP[x].clear();
            for (int y : prevOutList[x])
            {
                if (!UP[y].count(x)) continue;
                UP[y].erase(x);
                if (UP[y].empty() && level[y] != INF_LEVEL)
                {
                    LL[level[y]].insert(y);
                }
                else if (parent[y] == x && !UP[y].empty())
                {
                    parent[y] = *UP[y].begin();
                }
                
            }
            continue;
        }

        level[x] = newLevel;
        LL[level[x]].insert(x);

        UP[x].clear();
        for (int p : prevInList[x])
        {
            if (level[p] != INF_LEVEL &&
                level[p] == level[x] - 1)
            {
                UP[x].insert(p);
            }
        }
        // if (!UP[x].empty()) // will be done in next level
        // {
        //     parent[x] = *UP[x].begin();
        // }

        for (int y : prevOutList[x])
        {
            if (!UP[y].count(x)) continue;
            UP[y].erase(x);
            if (UP[y].empty() && level[y] != INF_LEVEL)
            {
                LL[level[y]].insert(y);
            }
            else if (parent[y] == x && !UP[y].empty())
            {
                parent[y] = *UP[y].begin();
            }
        }
    }
}

void fallback_BFS(int source, int n, vector<int>&level, vector<int>&parent,  vector<unordered_set<int>>& outAdj){
    //reset level and parent
    std::fill(level.begin(),  level.end(),  INF_LEVEL);
    std::fill(parent.begin(), parent.end(), NO_PARENT);

    level[source] = 0;
    std::queue<int> q;
    q.push(source);

    while (!q.empty()) {
        int u = q.front(); 
        q.pop();
        for (int v : outAdj[u]) {
            if (level[v] == INF_LEVEL) {
                level[v]  = level[u] + 1;
                parent[v] = u;
                q.push(v);
            }
        }
    }
}



// ------------------------------------------------
void repairLevel(BFSState& ws,
                        std::vector<std::set<int>>& LL,
                        int l)
{
    int n = ws.n;
    while (!LL[l].empty())
    {
        int x = *LL[l].begin();
        LL[l].erase(LL[l].begin());

        if (!ws.UP[x].empty())
        {
            ws.parent[x] = *ws.UP[x].begin();
            continue;
        }

        int newLevel = ws.level[x] + 1;

        if (newLevel > n)
        {
            ws.level[x] = INF_LEVEL;
            ws.parent[x] = NO_PARENT;
            ws.UP[x].clear();
            for (int y : ws.outAdj[x])
            {
                if (ws.UP[y].count(x))
                {
                    ws.UP[y].erase(x);
                    if (ws.UP[y].empty() && ws.level[y] != INF_LEVEL)
                    {
                        LL[ws.level[y]].insert(y);
                    }
                    else if (ws.parent[y] == x && !ws.UP[y].empty())
                    {
                        ws.parent[y] = *ws.UP[y].begin();
                    }
                }
            }
            continue;
        }

        ws.level[x] = newLevel;
        LL[ws.level[x]].insert(x);

        ws.UP[x].clear();
        for (int p : ws.inAdj[x])
        {
            if (ws.level[p] != INF_LEVEL &&
                ws.level[p] == ws.level[x] - 1)
            {
                ws.UP[x].insert(p);
            }
        }
        if (!ws.UP[x].empty())
        {
            ws.parent[x] = *ws.UP[x].begin();
        }

        for (int y : ws.outAdj[x])
        {
            if (!ws.UP[y].count(x)) continue;
            ws.UP[y].erase(x);
            if (ws.UP[y].empty() && ws.level[y] != INF_LEVEL)
            {
                LL[ws.level[y]].insert(y);
            }
            else if (ws.parent[y] == x && !ws.UP[y].empty())
            {
                ws.parent[y] = *ws.UP[y].begin();
            }
        }
    }
}


