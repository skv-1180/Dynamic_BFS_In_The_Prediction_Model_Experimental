#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../include/BFSState.h"
#include "../include/Config.h"
#include "../include/Decremental.h"
#include "../include/FullyDynamic.h"
#include "../include/Graph.h"
#include "../include/Incremental.h"
#include "../include/InitGraph.h"
#include "../include/Types.h"
#include "../include/timer.h"
#include "../include/utils.h"

const std::vector<std::string> KV_FLAGS = {"--mode"};
const std::vector<std::string> BOOL_FLAGS = {"--quiet", "--ec"};

void printResult(const QueryResult& r, int n, bool quiet);
bool isKVFlag(const std::string& s);
bool isBoolFlag(const std::string& s);

int main(int argc, char** argv)
{
    std::string modeStr = "fullydynamic";
    bool quiet = false;
    std::string inputFile;
    ErrorCorrectionMode errorCorrectionMode = ErrorCorrectionMode::TRIVIAL;

    for (int i = 1; i < argc; ++i)
    {
        std::string a(argv[i]);
        if (isKVFlag(a))
        {
            if (i + 1 >= argc)
            {
                std::cerr << "Error: " << a << " requires a value\n";
                return 1;
            }
            std::string val(argv[++i]);
            if (a == "--mode")
                modeStr = val;
        }
        else if (isBoolFlag(a))
        {
            if (a == "--quiet")
                quiet = true;
            else if (a == "--ec")
                errorCorrectionMode = ErrorCorrectionMode::NONTRIVIAL;
        }
        else if (a.rfind("--", 0) == 0)
        {
            std::cerr << "Warning: unknown flag '" << a << "' ignored\n";
        }
        else
        {
            inputFile = a;
        }
    }

    AlgorithmMode mode = AlgorithmMode::FULLY_DYNAMIC;

    if (modeStr == "incremental")
        mode = AlgorithmMode::INCREMENTAL;
    else if (modeStr == "decremental")
        mode = AlgorithmMode::DECREMENTAL;

    Graph graph;
    try
    {
        if (inputFile.empty())
        {
            std::cout << "Reading from stdin...\n";
            graph = readGraph(std::cin);
        }
        else
        {
            graph = readGraphFromFile(inputFile);
        }
    }
    catch (const std::exception& ex)
    {
        std::cerr << "Input error: " << ex.what() << "\n";
        return 1;
    }

    if (!quiet)
    {
        std::cout << "=== Problem Instance ===\n";
        graph.print(std::cout);
        std::cout << "\n";
    }

    int n = graph.numVertices();
    int m = graph.numUpdates();
    int src = graph.source();

    int totalUpdates = 0;
    int caseOneCount = 0;
    int caseTwoCount = 0;
    bool allCorrect = true;

    BFSState realGraph{n, src};
    for (const auto& e : graph.initialEdges())
    {
        realGraph.addEdge(e.u, e.v);
    }

    Timer timer{true};
    if (mode == AlgorithmMode::INCREMENTAL)
    {
        if (!quiet)
            std::cout << "=== Incremental BFS with Predictions ===\n";

        IncrementalBFS algo(n, src,
                            graph.initialEdges(),
                            graph.predictedUpdates(), errorCorrectionMode);

        timer.reset();
        for (int j = 1; j <= m; ++j)
        {
            const auto& upd = graph.realUpdates()[j - 1];
            realGraph.addEdge(upd.u, upd.v);
            auto result = algo.processUpdate(j, upd, timer);
            printResult(result, n, quiet);
            ++totalUpdates;
            if (result.usedPrediction)
                ++caseOneCount;
            else
                ++caseTwoCount;
        }
    }
    else if (mode == AlgorithmMode::DECREMENTAL)
    {
        if (!quiet)
            std::cout << "=== Decremental BFS with Predictions ===\n";

        DecrementalBFS algo(n, src,
                            graph.initialEdges(),
                            graph.predictedUpdates(),
                            errorCorrectionMode);

        timer.reset();
        for (int j = 1; j <= m; ++j)
        {
            const auto& upd = graph.realUpdates()[j - 1];
            realGraph.removeEdge(upd.u, upd.v);
            auto result = algo.processUpdate(j, upd, timer);
            printResult(result, n, quiet);
            ++totalUpdates;
            if (result.usedPrediction)
                ++caseOneCount;
            else
                ++caseTwoCount;
        }
    }
    else
    {
        if (!quiet)
            std::cout << "=== Fully Dynamic BFS with Predictions ===\n";

        FullyDynamicBFS algo(n, src,
                             graph.initialEdges(),
                             graph.predictedUpdates(),
                             errorCorrectionMode);

        timer.reset();
        for (int j = 1; j <= m; ++j)
        {
            const auto& upd = graph.realUpdates()[j - 1];
            if (upd.type == UpdateType::DELETE)
            {
                realGraph.removeEdge(upd.u, upd.v);
            }
            else
            {
                realGraph.addEdge(upd.u, upd.v);
            }

            auto result = algo.processUpdate(j, upd, timer);
            printResult(result, n, quiet);
            ++totalUpdates;
            if (result.usedPrediction)
                ++caseOneCount;
            else
                ++caseTwoCount;
        }
    }

    double ms = timer.elapsed_ms();
    std::cout << setprecision(3) << fixed;

    std::cout << "\n=== Summary ===\n";
    std::cout << "Mode          : " << modeStr << "\n";
    std::cout << "Total updates : " << totalUpdates << "\n";
    std::cout << "Case-1 (pred) : " << caseOneCount << "\n";
    std::cout << "Case-2 (batch): " << caseTwoCount << "\n";
    std::cout << "Time taken     : " << ms << " ms\n";

    return allCorrect ? 0 : 1;
}

// Helper function
void printResult(const QueryResult& r, int n, bool quiet)
{
    if (quiet) return;
    std::cout << "After update " << r.step
              << (r.usedPrediction ? " [PRED]" : " [BATCH]")
              << "  (last match=" << r.lastMatchedStep << ")\n";
    std::cout << "  Vertex | Level | Parent\n";
    for (int v = 1; v <= n; ++v)
    {
        std::cout << "  " << v << " | ";
        if (r.level[v] == INF_LEVEL)
            std::cout << "INF | ";
        else
            std::cout << r.level[v] << "   | ";
        if (r.parent[v] == NO_PARENT)
            std::cout << "none\n";
        else
            std::cout << r.parent[v] << "\n";
    }
}

bool isKVFlag(const std::string& s)
{
    for (auto& f : KV_FLAGS)
        if (s == f) return true;
    return false;
}

bool isBoolFlag(const std::string& s)
{
    for (auto& f : BOOL_FLAGS)
        if (s == f) return true;
    return false;
}
