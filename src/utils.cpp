#include "../include/utils.h"

#include <algorithm>
#include <cassert>
#include <queue>
#include <set>
#include <vector>

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

