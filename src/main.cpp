#include <algorithm>
#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>

#include "../include/BFSAlgorithms.h"
#include "../include/BFSState.h"
#include "../include/Config.h"
#include "../include/Decremental.h"
#include "../include/FullyDynamic.h"
#include "../include/Graph.h"
#include "../include/Incremental.h"
#include "../include/InitGraph.h"
#include "../include/Types.h"

static void printResult(const QueryResult& r, int n, bool quiet) {
    if (quiet) return;
    std::cout << "After update " << r.step
              << (r.usedPrediction ? " [PRED]" : " [BATCH]")
              << "  (last match=" << r.lastMatchedStep << ")\n";
    std::cout << "  Vertex | Level | Parent\n";
    for (int v = 1; v <= n; ++v) {
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

static bool verifyResult(const QueryResult& r,
                         const BFSState& realGraph,
                         int n) {
    BFSState check(n, realGraph.source);
    check.outAdj = realGraph.outAdj;
    check.inAdj = realGraph.inAdj;
    check.computeBFS();

    bool ok = true;
    for (int v = 1; v <= n; ++v) {
        if (r.level[v] != check.level[v]) {
            std::cerr << "[MISMATCH] step=" << r.step
                      << " v=" << v
                      << " got=" << r.level[v]
                      << " expected=" << check.level[v] << "\n";
            ok = false;
        }
    }
    return ok;
}

static const std::vector<std::string> KV_FLAGS = {"--mode", "--store"};
static const std::vector<std::string> BOOL_FLAGS = {"--verify", "--quiet"};

static bool isKVFlag(const std::string& s) {
    for (auto& f : KV_FLAGS)
        if (s == f) return true;
    return false;
}
static bool isBoolFlag(const std::string& s) {
    for (auto& f : BOOL_FLAGS)
        if (s == f) return true;
    return false;
}

int main(int argc, char** argv) {
    std::string modeStr = "fullydynamic";
    std::string storeStr = "full";
    bool doVerify = false;
    bool quiet = false;
    std::string inputFile;

    for (int i = 1; i < argc; ++i) {
        std::string a(argv[i]);
        if (isKVFlag(a)) {
            if (i + 1 >= argc) {
                std::cerr << "Error: " << a << " requires a value\n";
                return 1;
            }
            std::string val(argv[++i]);
            if (a == "--mode")
                modeStr = val;
            else if (a == "--store")
                storeStr = val;
        } else if (isBoolFlag(a)) {
            if (a == "--verify")
                doVerify = true;
            else if (a == "--quiet")
                quiet = true;
        } else if (a.rfind("--", 0) == 0) {
            std::cerr << "Warning: unknown flag '" << a << "' ignored\n";
        } else {
            inputFile = a;
        }
    }

    AlgorithmMode mode = AlgorithmMode::FULLY_DYNAMIC;
    if (modeStr == "incremental")
        mode = AlgorithmMode::INCREMENTAL;
    else if (modeStr == "decremental")
        mode = AlgorithmMode::DECREMENTAL;

    StorageMode storeMode = StorageMode::FULL_SNAPSHOTS;
    if (storeStr != "full") {
        std::cerr << "Warning: unsupported storage '" << storeStr
                  << "' ignored; using 'full'\n";
        storeStr = "full";
    }

    Graph graph;
    try {
        if (inputFile.empty()) {
            std::cout << "Reading from stdin...\n";
            graph = readGraph(std::cin);
        } else {
            graph = readGraphFromFile(inputFile);
        }
    } catch (const std::exception& ex) {
        std::cerr << "Input error: " << ex.what() << "\n";
        return 1;
    }

    if (!quiet) {
        std::cout << "=== Problem Instance ===\n";
        graph.print(std::cout);
        std::cout << "\n";
    }

    int n = graph.numVertices();
    int m = graph.numUpdates();
    int src = graph.source();
    (void)storeMode;

    auto t0 = std::chrono::high_resolution_clock::now();

    int totalUpdates = 0;
    int caseOneCount = 0;
    int caseTwoCount = 0;
    bool allCorrect = true;

    if (mode == AlgorithmMode::INCREMENTAL) {
        if (!quiet)
            std::cout << "=== Incremental BFS with Predictions ===\n";

        IncrementalBFS algo(n, src,
                            graph.initialEdges(),
                            graph.predictedUpdates());

        for (int j = 1; j <= m; ++j) {
            const auto& upd = graph.realUpdates()[j - 1];
            auto result = algo.processUpdate(j, upd);
            printResult(result, n, quiet);
            ++totalUpdates;
            if (result.usedPrediction)
                ++caseOneCount;
            else
                ++caseTwoCount;

            if (doVerify && !verifyResult(result, algo.realGraph(), n)) {
                allCorrect = false;
                std::cerr << "  -> CORRECTNESS FAILURE at step " << j << "\n";
            }
        }
    } else if (mode == AlgorithmMode::DECREMENTAL) {
        if (!quiet)
            std::cout << "=== Decremental BFS with Predictions ===\n";

        DecrementalBFS algo(n, src,
                            graph.initialEdges(),
                            graph.predictedUpdates());

        for (int j = 1; j <= m; ++j) {
            const auto& upd = graph.realUpdates()[j - 1];
            auto result = algo.processUpdate(j, upd);
            printResult(result, n, quiet);
            ++totalUpdates;
            if (result.usedPrediction)
                ++caseOneCount;
            else
                ++caseTwoCount;

            if (doVerify && !verifyResult(result, algo.realGraph(), n)) {
                allCorrect = false;
                std::cerr << "  -> CORRECTNESS FAILURE at step " << j << "\n";
            }
        }
    } else {
        if (!quiet)
            std::cout << "=== Fully Dynamic BFS with Predictions ===\n";

        FullyDynamicBFS algo(n, src,
                             graph.initialEdges(),
                             graph.predictedUpdates());

        for (int j = 1; j <= m; ++j) {
            const auto& upd = graph.realUpdates()[j - 1];
            auto result = algo.processUpdate(j, upd);
            printResult(result, n, quiet);
            ++totalUpdates;
            if (result.usedPrediction)
                ++caseOneCount;
            else
                ++caseTwoCount;

            if (doVerify && !verifyResult(result, algo.realGraph(), n)) {
                allCorrect = false;
                std::cerr << "  -> CORRECTNESS FAILURE at step " << j << "\n";
            }
        }
    }

    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::cout << "\n=== Summary ===\n";
    std::cout << "Mode          : " << modeStr << "\n";
    std::cout << "Storage       : " << storeStr << "\n";
    std::cout << "Total updates : " << totalUpdates << "\n";
    std::cout << "Case-1 (pred) : " << caseOneCount << "\n";
    std::cout << "Case-2 (batch): " << caseTwoCount << "\n";
    std::cout << "Wall time     : " << ms << " ms\n";
    if (doVerify)
        std::cout << "Correctness   : " << (allCorrect ? "PASS" : "FAIL") << "\n";

    return allCorrect ? 0 : 1;
}
