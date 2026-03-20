// ============================================================
// BFSAlgorithms.cpp
// ============================================================

#include "../include/BFSAlgorithms.h"
#include <queue>
#include <vector>
#include <set>
#include <algorithm>
#include <cassert>

// ===========================================================
// Helpers
// ===========================================================
static inline bool levelInfOrGreater(int a, int b)
{
    // safe comparison when a or b may be INF_LEVEL
    if (a == INF_LEVEL) return false; // a is inf, can't be > anything useful
    if (b == INF_LEVEL) return true;  // b is inf, a+1 < inf
    return a + 1 < b; // i.e. level[u]+1 < level[v]
}

// ===========================================================
// Classical single-edge incremental BFS (ES-Tree)
// Appendix A of the paper – Algorithm classicalInsertEdge
// ===========================================================
void classicalInsertEdge(BFSState& state, int u, int v)
{
    state.addEdge(u, v);

    // If v's level cannot be improved, nothing to do
    if (state.level[u] == INF_LEVEL) return;
    if (state.level[v] <= state.level[u] + 1) return;

    int n = state.n;
    std::vector<std::vector<int>> LR(n + 2);

    state.parent[v] = u;
    state.level[v]  = state.level[u] + 1;
    LR[state.level[v]].push_back(v);

    // Propagate level decreases level-by-level
    for (int l = state.level[v]; l <= n; ++l) {
        for (int idx = 0; idx < (int)LR[l].size(); ++idx) {
            int y = LR[l][idx];
            for (int z : state.outAdj[y]) {
                if (state.level[z] > state.level[y] + 1) {
                    state.level[z]  = state.level[y] + 1;
                    state.parent[z] = y;
                    LR[state.level[z]].push_back(z);
                }
            }
        }
    }

    // Keep UP consistent for vertices whose level changed
    // (needed when this is used during decremental / fd preprocessing)
    for (int l = state.level[v]; l <= n; ++l) {
        for (int y : LR[l]) {
            // Recompute UP[y]
            state.UP[y].clear();
            for (int p : state.inAdj[y]) {
                if (state.level[p] != INF_LEVEL &&
                    state.level[p] == state.level[y] - 1)
                {
                    state.UP[y].insert(p);
                }
            }
        }
    }
}

// ===========================================================
// Classical single-edge decremental BFS (ES-Tree)
// Appendix A – Algorithm classicalDeleteEdge
// ===========================================================
void classicalDeleteEdge(BFSState& state, int u, int v)
{
    state.removeEdge(u, v);

    // Only the BFS tree needs repair if u was in UP[v]
    if (!state.UP[v].count(u)) return;
    state.UP[v].erase(u);

    if (!state.UP[v].empty()) {
        if (state.parent[v] == u)
            state.parent[v] = *state.UP[v].begin();
        return; // v still has an upper parent, no level change
    }

    int n = state.n;
    std::vector<std::set<int>> LL(n + 2);
    LL[state.level[v]].insert(v);

    for (int l = state.level[v]; l <= n; ++l) {
        while (!LL[l].empty()) {
            int x = *LL[l].begin();
            LL[l].erase(LL[l].begin());

            if (!state.UP[x].empty()) {
                state.parent[x] = *state.UP[x].begin();
                continue;
            }

            // x must go to the next level
            state.level[x]++;
            if (state.level[x] > n) {
                // vertex has become unreachable
                state.level[x]  = INF_LEVEL;
                state.parent[x] = NO_PARENT;
                state.UP[x].clear();
                // propagate unreachability to children
                for (int y : state.outAdj[x]) {
                    if (state.UP[y].count(x)) {
                        state.UP[y].erase(x);
                        if (state.UP[y].empty()) {
                            LL[state.level[y]].insert(y);
                        } else if (state.parent[y] == x) {
                            state.parent[y] = *state.UP[y].begin();
                        }
                    }
                }
                continue;
            }

            LL[state.level[x]].insert(x);

            // Recompute UP[x] at the new level
            state.UP[x].clear();
            for (int p : state.inAdj[x]) {
                if (state.level[p] != INF_LEVEL &&
                    state.level[p] == state.level[x] - 1)
                {
                    state.UP[x].insert(p);
                }
            }
            if (!state.UP[x].empty()) {
                state.parent[x] = *state.UP[x].begin();
            }

            // Notify x's children that x is no longer at the old level
            for (int y : state.outAdj[x]) {
                if (state.UP[y].count(x)) {
                    state.UP[y].erase(x);
                    if (state.UP[y].empty()) {
                        LL[state.level[y]].insert(y);
                    } else if (state.parent[y] == x) {
                        state.parent[y] = *state.UP[y].begin();
                    }
                }
            }
        }
    }
}

// ===========================================================
// buildWorkingState
// Reconstructs G_i's adjacency from G_j by undoing the batch
// updates in REVERSE order.
// ===========================================================
BFSState buildWorkingState(const BFSSnapshot& snap,
                           const BFSState&    realState,
                           const EdgeList&    batchUpdates)
{
    BFSState ws(realState.n, realState.source);

    // Start with G_j's adjacency
    ws.outAdj = realState.outAdj;
    ws.inAdj  = realState.inAdj;

    // Undo batch updates in reverse to reconstruct G_i
    for (int k = (int)batchUpdates.size() - 1; k >= 0; --k) {
        const auto& e = batchUpdates[k];
        if (e.type == UpdateType::INSERT) {
            // Undo insertion = remove edge
            ws.outAdj[e.u].erase(e.v);
            ws.inAdj[e.v].erase(e.u);
        } else {
            // Undo deletion = add edge back
            ws.outAdj[e.u].insert(e.v);
            ws.inAdj[e.v].insert(e.u);
        }
    }

    // Load BFS data from snapshot[i]
    ws.loadSnapshot(snap);
    return ws;
}

// ===========================================================
// batchInsertEdge  (Section 3)
// ws.outAdj already contains G_i's adjacency.
// The batch edges will be added inside this function.
// ===========================================================
void batchInsertEdge(BFSState& ws, const EdgeList& batch)
{
    int n = ws.n;
    std::vector<std::vector<int>> LR(n + 2);
    std::vector<bool>             vis(n + 1, false);
    int lStar = n + 1;

    // Phase 1: add batch edges and find initial level improvements
    for (const auto& e : batch) {
        if (e.type != UpdateType::INSERT) continue;

        // Apply edge to adjacency
        ws.outAdj[e.u].insert(e.v);
        ws.inAdj[e.v].insert(e.u);

        if (ws.level[e.u] == INF_LEVEL) continue;
        if (ws.level[e.v] > ws.level[e.u] + 1) {
            ws.parent[e.v] = e.u;
            ws.level[e.v]  = ws.level[e.u] + 1;
            LR[ws.level[e.v]].push_back(e.v);
            lStar = std::min(lStar, ws.level[e.v]);
        }
    }

    if (lStar > n) return; // no changes

    // Phase 2: propagate level decreases level-by-level
    for (int l = lStar; l <= n; ++l) {
        for (int idx = 0; idx < (int)LR[l].size(); ++idx) {
            int y = LR[l][idx];
            if (vis[y]) continue;
            vis[y] = true;

            for (int z : ws.outAdj[y]) {
                if (ws.level[z] > ws.level[y] + 1) {
                    ws.level[z]  = ws.level[y] + 1;
                    ws.parent[z] = y;
                    LR[ws.level[z]].push_back(z);
                }
            }
        }
    }
}

// ===========================================================
// batchDeleteEdge  (Section 4)
// ws.outAdj / ws.inAdj starts as G_i's adjacency.
// ws.UP must be loaded from snapshot[i].
// ===========================================================

// Forward declaration for RepairLevel (shared with batchDynamicUpdate)
static void repairLevel(BFSState& ws,
                        std::vector<std::set<int>>& LL,
                        int l);

void batchDeleteEdge(BFSState& ws, const EdgeList& batch)
{
    int n = ws.n;
    std::vector<std::set<int>> LL(n + 2);
    int lStar = n + 1;

    // PreprocessDeletions
    for (const auto& e : batch) {
        if (e.type != UpdateType::DELETE) continue;

        ws.outAdj[e.u].erase(e.v);
        ws.inAdj[e.v].erase(e.u);

        if (!ws.UP[e.v].count(e.u)) continue;
        ws.UP[e.v].erase(e.u);

        if (ws.UP[e.v].empty()) {
            if (ws.level[e.v] != INF_LEVEL) {
                LL[ws.level[e.v]].insert(e.v);
                lStar = std::min(lStar, ws.level[e.v]);
            }
        } else if (ws.parent[e.v] == e.u) {
            ws.parent[e.v] = *ws.UP[e.v].begin();
        }
    }

    // RepairLevel for each level l* .. n
    for (int l = lStar; l <= n; ++l) {
        repairLevel(ws, LL, l);
    }
}

// ===========================================================
// batchDynamicUpdate  (Section 5)
// ===========================================================

// RepairLevel helper: processes L_L[l] and handles vertices
// that must increase their level.
static void repairLevel(BFSState& ws,
                        std::vector<std::set<int>>& LL,
                        int l)
{
    int n = ws.n;
    while (!LL[l].empty()) {
        int x = *LL[l].begin();
        LL[l].erase(LL[l].begin());

        if (!ws.UP[x].empty()) {
            // Found a replacement parent at level l-1
            ws.parent[x] = *ws.UP[x].begin();
            continue;
        }

        // x must increase level
        int newLevel = ws.level[x] + 1;

        if (newLevel > n) {
            // x becomes unreachable
            ws.level[x]  = INF_LEVEL;
            ws.parent[x] = NO_PARENT;
            ws.UP[x].clear();
            for (int y : ws.outAdj[x]) {
                if (ws.UP[y].count(x)) {
                    ws.UP[y].erase(x);
                    if (ws.UP[y].empty() && ws.level[y] != INF_LEVEL) {
                        LL[ws.level[y]].insert(y);
                    } else if (ws.parent[y] == x && !ws.UP[y].empty()) {
                        ws.parent[y] = *ws.UP[y].begin();
                    }
                }
            }
            continue;
        }

        ws.level[x] = newLevel;
        LL[ws.level[x]].insert(x); // enqueue at new level

        // Recompute UP[x] at the new level
        ws.UP[x].clear();
        for (int p : ws.inAdj[x]) {
            if (ws.level[p] != INF_LEVEL &&
                ws.level[p] == ws.level[x] - 1)
            {
                ws.UP[x].insert(p);
            }
        }
        // Assign parent if UP is now non-empty
        if (!ws.UP[x].empty()) {
            ws.parent[x] = *ws.UP[x].begin();
        }

        // Notify children: x's old level was l, new level is newLevel.
        // Remove x from UP[y] for each y in Out[x] that had x as upper parent.
        for (int y : ws.outAdj[x]) {
            if (!ws.UP[y].count(x)) continue;
            ws.UP[y].erase(x);
            if (ws.UP[y].empty() && ws.level[y] != INF_LEVEL) {
                LL[ws.level[y]].insert(y);
            } else if (ws.parent[y] == x && !ws.UP[y].empty()) {
                ws.parent[y] = *ws.UP[y].begin();
            }
        }
    }
}

void batchDynamicUpdate(BFSState& ws,
                        const EdgeList& deletes,
                        const EdgeList& inserts)
{
    int n = ws.n;
    std::vector<std::vector<int>> LR(n + 2);
    std::vector<std::set<int>>    LL(n + 2);
    int lStar = n + 1;

    // ---------------------------------------------------------
    // 1. PreprocessDeletions
    // ---------------------------------------------------------
    for (const auto& e : deletes) {
        ws.outAdj[e.u].erase(e.v);
        ws.inAdj[e.v].erase(e.u);

        if (!ws.UP[e.v].count(e.u)) continue;
        ws.UP[e.v].erase(e.u);

        if (ws.UP[e.v].empty()) {
            if (ws.level[e.v] != INF_LEVEL) {
                LL[ws.level[e.v]].insert(e.v);
                lStar = std::min(lStar, ws.level[e.v]);
            }
        } else if (ws.parent[e.v] == e.u) {
            ws.parent[e.v] = *ws.UP[e.v].begin();
        }
    }

    // ---------------------------------------------------------
    // 2. Process insertions and find initial level decreases
    // ---------------------------------------------------------
    for (const auto& e : inserts) {
        ws.outAdj[e.u].insert(e.v);
        ws.inAdj[e.v].insert(e.u);

        if (ws.level[e.u] == INF_LEVEL) continue;

        if (ws.level[e.v] > ws.level[e.u] + 1) {
            ws.parent[e.v] = e.u;
            ws.level[e.v]  = ws.level[e.u] + 1;
            ws.UP[e.v]     = {e.u};
            LR[ws.level[e.v]].push_back(e.v);
            lStar = std::min(lStar, ws.level[e.v]);
        } else if (ws.level[e.v] == ws.level[e.u] + 1) {
            ws.UP[e.v].insert(e.u);
        }
    }

    if (lStar > n) return;

    // ---------------------------------------------------------
    // 3. Sweep levels from lStar upwards
    // ---------------------------------------------------------
    for (int l = lStar; l <= n; ++l) {
        // (a) Repair deletion-driven level increases at this level
        repairLevel(ws, LL, l);

        // (b) Process insertion-driven level decreases at this level
        for (int idx = 0; idx < (int)LR[l].size(); ++idx) {
            int y = LR[l][idx];
            for (int z : ws.outAdj[y]) {
                // Case: z was about to increase its level (in LL[l+1])
                // A shorter path rescues z.
                if (LL[l + 1].count(z)) {
                    LL[l + 1].erase(z);
                    ws.parent[z] = y;
                    ws.level[z]  = l + 1; // level unchanged
                    // Add y to UP[z] (it was empty)
                    ws.UP[z].insert(y);
                    continue;
                }
                // Case: z's level can be improved
                if (ws.level[z] > ws.level[y] + 1) {
                    ws.level[z]  = ws.level[y] + 1;
                    ws.parent[z] = y;
                    ws.UP[z]     = {y};
                    LR[ws.level[z]].push_back(z);
                } else if (ws.level[z] == ws.level[y] + 1) {
                    ws.UP[z].insert(y);
                }
            }
        }
    }
}
