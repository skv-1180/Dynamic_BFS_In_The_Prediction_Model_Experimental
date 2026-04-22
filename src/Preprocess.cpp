#include "../include/Preprocess.h"

static BFSState buildInitialState(int numVertices, int source,
                                  const EdgeList& initialEdges)
{
    BFSState state(numVertices, source);
    for (const auto& e : initialEdges) {
        state.addEdge(e.u, e.v);
    }
    state.computeBFS();
    return state;
}

void classicalInsertEdge(BFSState& state, int u, int v)
{
    state.addEdge(u, v);

    if (state.level[u] == INF_LEVEL) return;
    if (state.level[v] <= state.level[u] + 1) return;

    int n = state.n;
    std::vector<std::vector<int>> LR(n + 2);

    state.parent[v] = u;
    state.level[v] = state.level[u] + 1;
    LR[state.level[v]].push_back(v);

    for (int l = state.level[v]; l <= n; ++l)
    {
        for (int idx = 0; idx < (int)LR[l].size(); ++idx)
        {
            int y = LR[l][idx];
            for (int z : state.outAdj[y])
            {
                if (state.level[z] > state.level[y] + 1)
                {
                    state.level[z] = state.level[y] + 1;
                    state.parent[z] = y;
                    LR[state.level[z]].push_back(z);
                }
            }
        }
    }

    // Keep UP consistent for vertices whose level changed
    // (needed when this is used during decremental / fd preprocessing)
    for (int l = state.level[v]; l <= n; ++l)
    {
        for (int y : LR[l])
        {
            // Recompute UP[y]
            state.UP[y].clear();
            for (int p : state.inAdj[y])
            {
                if (state.level[p] != INF_LEVEL &&
                    state.level[p] == state.level[y] - 1)
                {
                    state.UP[y].insert(p);
                }
            }
        }
    }
}

void classicalDeleteEdge(BFSState& state, int u, int v)
{
    state.removeEdge(u, v);

    if (!state.UP[v].count(u)) return;
    state.UP[v].erase(u);

    if (!state.UP[v].empty())
    {
        if (state.parent[v] == u)
            state.parent[v] = *state.UP[v].begin();
        return;
    }

    int n = state.n;
    std::vector<std::set<int>> LL(n + 2);
    LL[state.level[v]].insert(v);

    for (int l = state.level[v]; l <= n; ++l)
    {
        while (!LL[l].empty())
        {
            int x = *LL[l].begin();
            LL[l].erase(LL[l].begin());

            if (!state.UP[x].empty())
            {
                state.parent[x] = *state.UP[x].begin();
                continue;
            }

            state.level[x]++;
            if (state.level[x] > n)
            {
                state.level[x] = INF_LEVEL;
                state.parent[x] = NO_PARENT;
                state.UP[x].clear();
                for (int y : state.outAdj[x])
                {
                    if (state.UP[y].count(x))
                    {
                        state.UP[y].erase(x);
                        if (state.UP[y].empty())
                        {
                            LL[state.level[y]].insert(y);
                        }
                        else if (state.parent[y] == x)
                        {
                            state.parent[y] = *state.UP[y].begin();
                        }
                    }
                }
                continue;
            }

            LL[state.level[x]].insert(x);

            state.UP[x].clear();
            for (int p : state.inAdj[x])
            {
                if (state.level[p] != INF_LEVEL &&
                    state.level[p] == state.level[x] - 1)
                {
                    state.UP[x].insert(p);
                }
            }
            if (!state.UP[x].empty())
            {
                state.parent[x] = *state.UP[x].begin();
            }

            for (int y : state.outAdj[x])
            {
                if (state.UP[y].count(x))
                {
                    state.UP[y].erase(x);
                    if (state.UP[y].empty())
                    {
                        LL[state.level[y]].insert(y);
                    }
                    else if (state.parent[y] == x)
                    {
                        state.parent[y] = *state.UP[y].begin();
                    }
                }
            }
        }
    }
}

std::vector<BFSSnapshot> preprocessIncremental(
    int             numVertices,
    int             source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates)
{
    BFSState state = buildInitialState(numVertices, source, initialEdges);
    int m = (int)predictedUpdates.size();
    std::vector<BFSSnapshot> snapshots(m + 1);
    snapshots[0] = state.saveSnapshot(/*saveUP=*/false);

    for (int i = 0; i < m; ++i) {
        const auto& e = predictedUpdates[i];
        // All updates must be insertions in the incremental setting
        if (e.type == UpdateType::INSERT) {
            classicalInsertEdge(state, e.u, e.v);
        }
        // If a deletion appears in a supposedly incremental sequence,
        // we skip it (defensive programming).
        snapshots[i + 1] = state.saveSnapshot(false); //saveUP
    }
    return snapshots;
}

std::vector<BFSSnapshot> preprocessDecremental(
    int             numVertices,
    int             source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates)
{
    BFSState state = buildInitialState(numVertices, source, initialEdges);
    state.computeAllUP(); // UP sets required for decremental repair

    int m = (int)predictedUpdates.size();
    std::vector<BFSSnapshot> snapshots(m + 1);
    snapshots[0] = state.saveSnapshot(true); // saveUP

    for (int i = 0; i < m; ++i) {
        const auto& e = predictedUpdates[i];
        if (e.type == UpdateType::DELETE) {
            classicalDeleteEdge(state, e.u, e.v);
        }
        snapshots[i + 1] = state.saveSnapshot(true); //saveUP
    }
    return snapshots;
}

std::vector<BFSSnapshot> preprocessFullyDynamic(
    int             numVertices,
    int             source,
    const EdgeList& initialEdges,
    const EdgeList& predictedUpdates)
{
    BFSState state = buildInitialState(numVertices, source, initialEdges);
    state.computeAllUP();

    int m = (int)predictedUpdates.size();
    std::vector<BFSSnapshot> snapshots(m + 1);
    snapshots[0] = state.saveSnapshot(/*saveUP=*/true);

    for (int i = 0; i < m; ++i) {
        const auto& e = predictedUpdates[i];
        if (e.type == UpdateType::INSERT) {
            state.addEdge(e.u, e.v);
        } else {
            state.removeEdge(e.u, e.v);
        }
        state.computeBFS();
        state.computeAllUP();
        snapshots[i + 1] = state.saveSnapshot(true);
    }
    return snapshots;
}
