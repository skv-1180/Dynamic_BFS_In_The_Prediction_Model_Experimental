#include "ClassicalBFS.h"

#include <algorithm>
#include <chrono>
#include <limits>
#include <queue>
#include <vector>

using Clock = std::chrono::high_resolution_clock;
using Micros = std::chrono::duration<double, std::micro>;
static inline double elapsed_us(Clock::time_point t0, Clock::time_point t1)
{
    return Micros(t1 - t0).count();
}

static void insert_edge(BFSState& s, int x, int y)
{
    s.addEdge(x, y);
    if (s.level[x] == INF_LEVEL) return;
    if (s.level[y] <= s.level[x] + 1) return;

    int n = s.n;
    s.parent[y] = x;
    s.level[y] = s.level[x] + 1;

    std::vector<std::vector<int>> L(n + 2);
    L[s.level[y]].push_back(y);

    for (int i = s.level[y]; i <= n; ++i)
    {
        if (L[i].empty()) break;
        for (int a : L[i])
        {
            for (int v : s.outAdj[a])
            {  
                if (s.level[v] > s.level[a] + 1)
                {
                    s.parent[v] = a;
                    s.level[v] = s.level[a] + 1;
                    L[s.level[v]].push_back(v);
                }
            }
        }
    }
}

static void remove_edge(BFSState& s,
                        std::vector<std::vector<int>>& A, int u, int v)
{
    s.removeEdge(u, v);
    auto& Av = A[v];
    Av.erase(std::remove(Av.begin(), Av.end(), u), Av.end());

    if (s.parent[v] != u) return;  

    int n = s.n;

    std::vector<std::vector<int>> Li(n + 2);
    std::queue<int> q;
    q.push(v);
    while (!q.empty())
    {
        int w = q.front();
        q.pop();
        if (s.level[w] != INF_LEVEL && s.level[w] <= n)
            Li[s.level[w]].push_back(w);
        // Children in BFS tree = out-neighbours of w whose parent is w
        for (int c : s.outAdj[w])
            if (s.parent[c] == w) q.push(c);
    }

    // Set dist(w) = ∞ for all w ∈ T(v)
    for (int i = 1; i <= n; ++i)
        for (int w : Li[i])
        {
            s.level[w] = INF_LEVEL;
            s.parent[w] = NO_PARENT;
        }

    for (int i = 1; i <= n; ++i)
    {
        for (int ji = 0; ji < (int)Li[i].size(); ++ji)
        {
            int y = Li[i][ji];
            auto& Ay = A[y];
            bool found = false;

            std::size_t k = 0;
            while (k < Ay.size())
            {
                int x = Ay[k];
                if (s.level[x] >= i)
                {
                    // dist(x) >= i → not a valid parent at level i; remove
                    Ay.erase(Ay.begin() + (int)k);
                    // k unchanged — next element slid into position k
                }
                else
                {
                    // dist(x) < i → valid parent (dist(x) = i-1)
                    s.parent[y] = x;
                    s.level[y] = i;
                    found = true;
                    break;
                }
            }

            if (!found)
            {
                if (i < n) Li[i + 1].push_back(y);
                A[y].assign(s.inAdj[y].begin(), s.inAdj[y].end());
            }
        }
    }
}

AlgoTiming timeClassicalIncremental(
    const BFSState& initialState,
    const EdgeList& updates,
    int n_runs)
{
    AlgoTiming best;
    best.total_us = std::numeric_limits<double>::max();
    int m = (int)updates.size();

    for (int run = 0; run < n_runs; ++run)
    {
        BFSState state = initialState;  // copy not timed
        std::vector<double> per_upd(m);
        double run_total = 0.0;

        for (int j = 0; j < m; ++j)
        {
            const auto& e = updates[j];
            auto t0 = Clock::now();
            if (e.type == UpdateType::INSERT)
                insert_edge(state, e.u, e.v);
            auto t1 = Clock::now();
            per_upd[j] = elapsed_us(t0, t1);
            run_total += per_upd[j];
        }
        if (run_total < best.total_us)
        {
            best.total_us = run_total;
            best.per_update_us = per_upd;
        }
    }

    best.per_update_avg_us = (m > 0) ? best.total_us / m : 0.0;
    if (!best.per_update_us.empty())
    {
        best.per_update_min_us = *std::min_element(best.per_update_us.begin(), best.per_update_us.end());
        best.per_update_max_us = *std::max_element(best.per_update_us.begin(), best.per_update_us.end());
    }
    return best;
}

AlgoTiming timeClassicalDecremental(
    const BFSState& initialState,
    const EdgeList& updates,
    int n_runs)
{
    AlgoTiming best;
    best.total_us = std::numeric_limits<double>::max();
    int m = (int)updates.size();

    for (int run = 0; run < n_runs; ++run)
    {
        BFSState state = initialState;
        // Initialise A[v] = all current in-neighbours of v
        std::vector<std::vector<int>> A(state.n + 1);
        for (int v = 1; v <= state.n; ++v)
            A[v].assign(state.inAdj[v].begin(), state.inAdj[v].end());

        std::vector<double> per_upd(m);
        double run_total = 0.0;

        for (int j = 0; j < m; ++j)
        {
            const auto& e = updates[j];
            auto t0 = Clock::now();
            if (e.type == UpdateType::DELETE)
                remove_edge(state, A, e.u, e.v);
            auto t1 = Clock::now();
            per_upd[j] = elapsed_us(t0, t1);
            run_total += per_upd[j];
        }
        if (run_total < best.total_us)
        {
            best.total_us = run_total;
            best.per_update_us = per_upd;
        }
    }

    best.per_update_avg_us = (m > 0) ? best.total_us / m : 0.0;
    if (!best.per_update_us.empty())
    {
        best.per_update_min_us = *std::min_element(best.per_update_us.begin(), best.per_update_us.end());
        best.per_update_max_us = *std::max_element(best.per_update_us.begin(), best.per_update_us.end());
    }
    return best;
}

AlgoTiming timeClassicalFullyDynamic(
    const BFSState& initialState,
    const EdgeList& updates,
    int n_runs)
{
    AlgoTiming best;
    best.total_us = std::numeric_limits<double>::max();
    int m = (int)updates.size();

    for (int run = 0; run < n_runs; ++run)
    {
        BFSState state = initialState;
        std::vector<double> per_upd(m);
        double run_total = 0.0;

        for (int j = 0; j < m; ++j)
        {
            const auto& e = updates[j];
            auto t0 = Clock::now();
            if (e.type == UpdateType::INSERT)
                state.addEdge(e.u, e.v);
            else
                state.removeEdge(e.u, e.v);
            state.computeBFS();
            auto t1 = Clock::now();
            per_upd[j] = elapsed_us(t0, t1);
            run_total += per_upd[j];
        }
        if (run_total < best.total_us)
        {
            best.total_us = run_total;
            best.per_update_us = per_upd;
        }
    }

    best.per_update_avg_us = (m > 0) ? best.total_us / m : 0.0;
    if (!best.per_update_us.empty())
    {
        best.per_update_min_us = *std::min_element(best.per_update_us.begin(), best.per_update_us.end());
        best.per_update_max_us = *std::max_element(best.per_update_us.begin(), best.per_update_us.end());
    }
    return best;
}
