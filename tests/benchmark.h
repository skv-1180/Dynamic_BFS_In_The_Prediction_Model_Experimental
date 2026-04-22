#pragma once
#include "helper.h"

template <typename Algo>
RunMetrics collectMetricsAndEtaRows(
    Algo& algo,
    const EdgeList& init,
    const EdgeList& real,
    int n,
    const std::string& test_case,
    const std::string& graph_name,
    double error_rate,
    const std::string& algo_name,
    std::vector<EtaRow>& eta_rows,
    bool verify)
{
    RunMetrics rm;
    rm.total_updates = static_cast<int>(real.size());

    BFSState realGraph{n, algo.source()};
    for (const auto& e: init){
        realGraph.addEdge(e.u, e.v);
    }

    Timer timer(false);

    for (int j = 1; j <= static_cast<int>(real.size()); ++j) {
        const auto& upd = real[j - 1];
        if(upd.type == UpdateType::DELETE){
            realGraph.removeEdge(upd.u, upd.v);
        }else{
            realGraph.addEdge(upd.u, upd.v);
        }

        QueryResult r = algo.processUpdate(j, real[j - 1], timer);
        // if (verify && !verifyResult(r, realGraph, n)) {
        //     std::cerr << "Verification failed at step " << j << "\n";
        //     std::exit(1);
        // }

        if (r.usedPrediction) {
            ++rm.case1_count;
            continue;
        }

        ++rm.case2_count;

        const int i = r.lastMatchedStep;
        const int eta_e = j - i;

        const auto& snap = algo.snapshotAt(i);
        StepMetrics sm =
            computeStepMetrics(n, eta_e, snap.level, r.level, realGraph);

        rm.sum_eta_e += eta_e;
        rm.sum_eta_v += sm.eta_v;
        rm.sum_eta_v_star += sm.eta_v_star;

        rm.max_eta_e = std::max(rm.max_eta_e, static_cast<double>(eta_e));
        rm.max_eta_v = std::max(rm.max_eta_v, sm.eta_v);
        rm.max_eta_v_star = std::max(rm.max_eta_v_star, sm.eta_v_star);

        eta_rows.push_back(EtaRow{
            test_case,
            graph_name,
            error_rate,
            algo_name,
            j,
            i,
            eta_e,
            sm.eta_v,
            sm.eta_v_star
        });
    }

    rm.prediction_accuracy =
        (rm.total_updates > 0)
            ? static_cast<double>(rm.case1_count) / rm.total_updates
            : 1.0;

    if (rm.case2_count > 0) {
        rm.avg_eta_e = rm.sum_eta_e / rm.case2_count;
        rm.avg_eta_v = rm.sum_eta_v / rm.case2_count;
        rm.avg_eta_v_star = rm.sum_eta_v_star / rm.case2_count;
    }

    return rm;
}

template <typename Algo>
TimingResult timePredictedOnline(
    int n,
    int src,
    const EdgeList& init,
    const EdgeList& pred,
    const EdgeList& real,
    int runs,
    ErrorCorrectionMode errCorrMode
)
{
    TimingResult best;
    best.total_us = std::numeric_limits<double>::max();

    const int m = static_cast<int>(real.size());

    for (int run = 0; run < runs; ++run) {
        Algo algo(n, src, init, pred, errCorrMode);
        Timer timer(false);

        for (int j = 1; j <= m; ++j) {
            (void)algo.processUpdate(j, real[j - 1], timer);
        }

        const double total_us = timer.elapsed_us();
        if (total_us < best.total_us) {
            best.total_us = total_us;
        }
    }

    best.avg_us_per_update = (m > 0) ? best.total_us / m : 0.0;
    return best;
}

inline AlgoTiming timeClassical(
    AlgorithmMode mode,
    const BFSState& init_state,
    const EdgeList& real,
    int runs)
{
    if (mode == AlgorithmMode::INCREMENTAL) {
        return timeClassicalIncremental(init_state, real, runs);
    }
    if (mode == AlgorithmMode::DECREMENTAL) {
        return timeClassicalDecremental(init_state, real, runs);
    }
    return timeClassicalFullyDynamic(init_state, real, runs);
}
