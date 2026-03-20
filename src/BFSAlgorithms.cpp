#include "../include/BFSAlgorithms.h"

#include <algorithm>
#include <cassert>
#include <queue>
#include <set>
#include <vector>

static inline bool levelInfOrGreater(int a, int b) {
    if (a == INF_LEVEL) return false;
    if (b == INF_LEVEL) return true;
    return a + 1 < b;
}

void classicalInsertEdge(BFSState& state, int u, int v) {
    state.addEdge(u, v);

    if (state.level[u] == INF_LEVEL) return;
    if (state.level[v] <= state.level[u] + 1) return;

    int n = state.n;
    std::vector<std::vector<int>> LR(n + 2);

    state.parent[v] = u;
    state.level[v] = state.level[u] + 1;
    LR[state.level[v]].push_back(v);

    for (int l = state.level[v]; l <= n; ++l) {
        for (int idx = 0; idx < (int)LR[l].size(); ++idx) {
            int y = LR[l][idx];
            for (int z : state.outAdj[y]) {
                if (state.level[z] > state.level[y] + 1) {
                    state.level[z] = state.level[y] + 1;
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
                    state.level[p] == state.level[y] - 1) {
                    state.UP[y].insert(p);
                }
            }
        }
    }
}

void classicalDeleteEdge(BFSState& state, int u, int v) {
    state.removeEdge(u, v);

    if (!state.UP[v].count(u)) return;
    state.UP[v].erase(u);

    if (!state.UP[v].empty()) {
        if (state.parent[v] == u)
            state.parent[v] = *state.UP[v].begin();
        return;
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

            state.level[x]++;
            if (state.level[x] > n) {
                state.level[x] = INF_LEVEL;
                state.parent[x] = NO_PARENT;
                state.UP[x].clear();
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

            state.UP[x].clear();
            for (int p : state.inAdj[x]) {
                if (state.level[p] != INF_LEVEL &&
                    state.level[p] == state.level[x] - 1) {
                    state.UP[x].insert(p);
                }
            }
            if (!state.UP[x].empty()) {
                state.parent[x] = *state.UP[x].begin();
            }

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

BFSState buildWorkingState(const BFSSnapshot& snap,
                           const BFSState& realState,
                           const EdgeList& batchUpdates) {
    BFSState ws(realState.n, realState.source);

    ws.outAdj = realState.outAdj;
    ws.inAdj = realState.inAdj;

    for (int k = (int)batchUpdates.size() - 1; k >= 0; --k) {
        const auto& e = batchUpdates[k];
        if (e.type == UpdateType::INSERT) {
            ws.outAdj[e.u].erase(e.v);
            ws.inAdj[e.v].erase(e.u);
        } else {
            ws.outAdj[e.u].insert(e.v);
            ws.inAdj[e.v].insert(e.u);
        }
    }

    ws.loadSnapshot(snap);
    return ws;
}

void batchInsertEdge(BFSState& ws, const EdgeList& batch) {
    int n = ws.n;
    std::vector<std::vector<int>> LR(n + 2);
    std::vector<bool> vis(n + 1, false);
    int lStar = n + 1;

    for (const auto& e : batch) {
        if (e.type != UpdateType::INSERT) continue;

        ws.outAdj[e.u].insert(e.v);
        ws.inAdj[e.v].insert(e.u);

        if (ws.level[e.u] == INF_LEVEL) continue;
        if (ws.level[e.v] > ws.level[e.u] + 1) {
            ws.parent[e.v] = e.u;
            ws.level[e.v] = ws.level[e.u] + 1;
            LR[ws.level[e.v]].push_back(e.v);
            lStar = std::min(lStar, ws.level[e.v]);
        }
    }

    if (lStar > n) return;

    for (int l = lStar; l <= n; ++l) {
        for (int idx = 0; idx < (int)LR[l].size(); ++idx) {
            int y = LR[l][idx];
            if (vis[y]) continue;
            vis[y] = true;

            for (int z : ws.outAdj[y]) {
                if (ws.level[z] > ws.level[y] + 1) {
                    ws.level[z] = ws.level[y] + 1;
                    ws.parent[z] = y;
                    LR[ws.level[z]].push_back(z);
                }
            }
        }
    }
}

static void repairLevel(BFSState& ws,
                        std::vector<std::set<int>>& LL,
                        int l);

void batchDeleteEdge(BFSState& ws, const EdgeList& batch) {
    int n = ws.n;
    std::vector<std::set<int>> LL(n + 2);
    int lStar = n + 1;

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

    for (int l = lStar; l <= n; ++l) {
        repairLevel(ws, LL, l);
    }
}

static void repairLevel(BFSState& ws,
                        std::vector<std::set<int>>& LL,
                        int l) {
    int n = ws.n;
    while (!LL[l].empty()) {
        int x = *LL[l].begin();
        LL[l].erase(LL[l].begin());

        if (!ws.UP[x].empty()) {
            ws.parent[x] = *ws.UP[x].begin();
            continue;
        }

        int newLevel = ws.level[x] + 1;

        if (newLevel > n) {
            ws.level[x] = INF_LEVEL;
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
        LL[ws.level[x]].insert(x);

        ws.UP[x].clear();
        for (int p : ws.inAdj[x]) {
            if (ws.level[p] != INF_LEVEL &&
                ws.level[p] == ws.level[x] - 1) {
                ws.UP[x].insert(p);
            }
        }
        if (!ws.UP[x].empty()) {
            ws.parent[x] = *ws.UP[x].begin();
        }

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
                        const EdgeList& inserts) {
    int n = ws.n;
    std::vector<std::vector<int>> LR(n + 2);
    std::vector<std::set<int>> LL(n + 2);
    int lStar = n + 1;

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

    for (const auto& e : inserts) {
        ws.outAdj[e.u].insert(e.v);
        ws.inAdj[e.v].insert(e.u);

        if (ws.level[e.u] == INF_LEVEL) continue;

        if (ws.level[e.v] > ws.level[e.u] + 1) {
            ws.parent[e.v] = e.u;
            ws.level[e.v] = ws.level[e.u] + 1;
            ws.UP[e.v] = {e.u};
            LR[ws.level[e.v]].push_back(e.v);
            lStar = std::min(lStar, ws.level[e.v]);
        } else if (ws.level[e.v] == ws.level[e.u] + 1) {
            ws.UP[e.v].insert(e.u);
        }
    }

    if (lStar > n) return;

    for (int l = lStar; l <= n; ++l) {
        repairLevel(ws, LL, l);

        for (int idx = 0; idx < (int)LR[l].size(); ++idx) {
            int y = LR[l][idx];
            for (int z : ws.outAdj[y]) {
                if (LL[l + 1].count(z)) {
                    LL[l + 1].erase(z);
                    ws.parent[z] = y;
                    ws.level[z] = l + 1;
                    ws.UP[z].insert(y);
                    continue;
                }
                if (ws.level[z] > ws.level[y] + 1) {
                    ws.level[z] = ws.level[y] + 1;
                    ws.parent[z] = y;
                    ws.UP[z] = {y};
                    LR[ws.level[z]].push_back(z);
                } else if (ws.level[z] == ws.level[y] + 1) {
                    ws.UP[z].insert(y);
                }
            }
        }
    }
}
