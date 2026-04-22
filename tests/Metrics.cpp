#include "Metrics.h"

#include <algorithm>
#include <cmath>

StepMetrics computeStepMetrics(
    int n,
    int eta_e,
    const std::vector<int>& pred_level,
    const std::vector<int>& actual_level,
    const BFSState& graph_at_j)
{
    StepMetrics m;
    m.eta_e = eta_e;

    for (int v = 1; v <= n; ++v) {
        int lp = pred_level[v];
        int la = actual_level[v];

        // Treat INF as n+1 for arithmetic.
        if (lp == INF_LEVEL) lp = n + 1;
        if (la == INF_LEVEL) la = n + 1;

        const int err = std::abs(la - lp);
        if (err == 0) continue;

        const int deg =
            static_cast<int>(graph_at_j.inAdj[v].size() + graph_at_j.outAdj[v].size());

        m.eta_v += deg;
        m.eta_v_star += static_cast<double>(deg) * err;
    }

    return m;
}