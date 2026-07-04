#pragma once

#include <vector>
#include "../include/BFSState.h"

// Per case-2 event at step j, relative to last matched snapshot i.
struct StepMetrics {
    int eta_e{};                 
    double eta_v{};              
    double eta_v_star{};         
};

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

StepMetrics computeStepMetrics(
    int n,
    int eta_e,
    const std::vector<int>& pred_level,
    const std::vector<int>& actual_level,
    const BFSState& graph_at_j
);
