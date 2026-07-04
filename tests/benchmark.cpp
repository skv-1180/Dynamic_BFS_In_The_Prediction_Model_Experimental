#include "benchmark.h"
#include "helper.h"

int main(int argc, char** argv)
{
    Args args = parseArgs(argc, argv);

    AlgorithmMode mode = AlgorithmMode::FULLY_DYNAMIC;
    std::string algo_name = "fullydynamic";

    if (args.mode == "incremental")
    {
        mode = AlgorithmMode::INCREMENTAL;
        algo_name = "incremental";
    }
    else if (args.mode == "decremental")
    {
        mode = AlgorithmMode::DECREMENTAL;
        algo_name = "decremental";
    }
    else if (args.mode == "fullydynamic")
    {
        mode = AlgorithmMode::FULLY_DYNAMIC;
        algo_name = "fullydynamic";
    }
    else
    {
        std::cerr << "Invalid mode: " << args.mode << "\n";
        return 1;
    }

    Graph graph;
    try
    {
        graph = readGraphFromFile(args.input_file);
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Input error: " << ex.what() << "\n";
        return 1;
    }

    const int n = graph.numVertices();
    const int src = graph.source();
    const int m_updates = graph.numUpdates();

    const EdgeList& init = graph.initialEdges();
    const EdgeList& pred = graph.predictedUpdates();
    const EdgeList& real = graph.realUpdates();

    const std::string test_case = extractTestCaseName(args.input_file);
    const std::string graph_name = extractGraphName(test_case);
    const double error_rate = extractErrorRate(test_case);

    BFSState init_state(n, src);
    for (const auto& e : init)
    {
        init_state.addEdge(e.u, e.v);
    }
    init_state.computeBFS();
    init_state.computeAllUP();

    std::vector<EtaRow> eta_rows;
    RunMetrics rm;
    TimingResult online;
    AlgoTiming classical;
    ErrorCorrectionMode errorCorrMode = args.errorCorrectionMode;
    const std::string ec_label = ecToString(errorCorrMode);

    if (mode == AlgorithmMode::INCREMENTAL)
    {
        online = timePredictedOnline<IncrementalBFS>(
            n, src, init, pred, real, args.runs, errorCorrMode);
    }
    else if (mode == AlgorithmMode::DECREMENTAL)
    {
        online = timePredictedOnline<DecrementalBFS>(
            n, src, init, pred, real, args.runs, errorCorrMode);
    }
    else
    {
        online = timePredictedOnline<FullyDynamicBFS>(
            n, src, init, pred, real, args.runs, errorCorrMode);
    }

    classical = timeClassical(mode, init_state, real, args.runs);

    ensureCSVHeader(args.csv_time, timeCSVHeader());

    {
        std::ofstream out(args.csv_time, std::ios::app);
        if (!out)
        {
            std::cerr << "Cannot open " << args.csv_time << "\n";
            return 1;
        }

        appendTimeRow(
            out,
            test_case,
            graph_name,
            error_rate,
            algo_name,
            ec_label,
            n,
            static_cast<int>(init.size()),
            m_updates,
            rm,
            online,
            classical);
    }

    const double speedup =
        (online.total_us > 1e-12) ? classical.total_us / online.total_us : 0.0;

    std::cout << "Benchmark complete\n";
    std::cout << "  testcase            : " << test_case << "\n";
    std::cout << "  algorithm           : " << algo_name << "\n";
    std::cout << "  online_total_us     : " << ff(online.total_us) << "\n";
    std::cout << "  classical_total_us  : " << ff(classical.total_us) << "\n";
    std::cout << "  wrote               : " << args.csv_time << "\n";

    return 0;
}