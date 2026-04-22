#pragma once
#include <vector>
#include <limits>
#include "../include/BFSState.h"
#include "../include/Types.h"

struct AlgoTiming {
    double total_us{0};           // best total time (µs)
    double per_update_avg_us{0};  // total_us / num_updates
    double per_update_min_us{std::numeric_limits<double>::max()};
    double per_update_max_us{0};
    std::vector<double> per_update_us; // individual update times (best run)
};

AlgoTiming timeClassicalIncremental(
    const BFSState& initialState,
    const EdgeList& updates,
    int n_runs = 5
);

AlgoTiming timeClassicalDecremental(
    const BFSState& initialState,
    const EdgeList& updates,
    int n_runs = 5
);

AlgoTiming timeClassicalFullyDynamic(
    const BFSState& initialState,
    const EdgeList& updates,
    int n_runs = 5
);

AlgoTiming timeNaiveFullyDynamic(
    const BFSState& initialState,
    const EdgeList& updates,
    int n_runs = 5
);
