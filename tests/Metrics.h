#pragma once

#include <vector>
#include "../include/BFSState.h"

// Per case-2 event at step j, relative to last matched snapshot i.
struct StepMetrics {
    int eta_e{};                 // j - i
    double eta_v{};              // Σ deg(v) over vertices with wrong level
    double eta_v_star{};         // Σ deg(v) * |L_j[v] - Lhat_i[v]|
};

// Aggregate statistics over one full testcase run.
struct RunMetrics {
    int total_updates{};
    int case1_count{};
    int case2_count{};
    double prediction_accuracy{};

    double sum_eta_e{};
    double sum_eta_v{};
    double sum_eta_v_star{};

    double avg_eta_e{};
    double avg_eta_v{};
    double avg_eta_v_star{};

    double max_eta_e{};
    double max_eta_v{};
    double max_eta_v_star{};
};

// Compute metrics for one case-2 event.
// pred_level and actual_level are 1-indexed vectors of size n+1.
StepMetrics computeStepMetrics(
    int n,
    int eta_e,
    const std::vector<int>& pred_level,
    const std::vector<int>& actual_level,
    const BFSState& graph_at_j
);
