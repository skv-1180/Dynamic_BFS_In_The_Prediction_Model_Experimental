#pragma once

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include "ClassicalBFS.h"
#include "../include/BFSState.h"
#include "../include/Config.h"
#include "../include/Decremental.h"
#include "../include/FullyDynamic.h"
#include "../include/Graph.h"
#include "../include/Incremental.h"
#include "../include/InitGraph.h"
#include "../include/Preprocess.h"
#include "../include/Types.h"
#include "../include/utils.h"
#include "Metrics.h"

using Clock = std::chrono::high_resolution_clock;
using Micros = std::chrono::duration<double, std::micro>;

struct Args
{
    std::string mode = "fullydynamic";
    int runs = 5;
    bool verify = false;
    std::string csv_time = "time_vs_error.csv";
    std::string csv_eta = "eta_scatter.csv";
    std::string input_file;
    ErrorCorrectionMode errorCorrectionMode = ErrorCorrectionMode::TRIVIAL;
};

struct TimingResult
{
    double total_us = 0.0;
    double avg_us_per_update = 0.0;
};

struct EtaRow
{
    std::string test_case;
    std::string graph_name;
    double error_rate = -1.0;
    std::string algo;
    int step_j = 0;
    int last_match_i = 0;
    int eta_e = 0;
    double eta_v = 0.0;
    double eta_v_star = 0.0;
};
std::string ff(double v, int p = 4);

inline std::string ecToString(ErrorCorrectionMode m)
{
    if (m == ErrorCorrectionMode::NONTRIVIAL) return "ec";
    return "noec";
}

std::string extractTestCaseName(const std::string& path);

double extractErrorRate(const std::string& test_case);

std::string extractGraphName(const std::string& test_case);

std::string timeCSVHeader();

std::string etaCSVHeader();

Args parseArgs(int argc, char** argv);
bool verifyResult(const QueryResult& r, const BFSState& real, int n);
void ensureCSVHeader(const std::string& path, const std::string& header);
void appendTimeRow(std::ofstream& out,
                   const std::string& test_case,
                   const std::string& graph_name,
                   double error_rate,
                   const std::string& algo,
                   const std::string& ec,
                   int n,
                   int m_init,
                   int m_updates,
                   const RunMetrics& rm,
                   const TimingResult& online,
                   const AlgoTiming& classical);

void appendEtaRows(std::ofstream& out, const std::vector<EtaRow>& rows, const std::string& ec_label);

