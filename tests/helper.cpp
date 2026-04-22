#include "helper.h"

std::string ff(double v, int p) {
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(p) << v;
    return ss.str();
}

std::string extractTestCaseName(const std::string& path) {
    return std::filesystem::path(path).stem().string();
}

double extractErrorRate(const std::string& test_case) {
    auto pos = test_case.find("err");
    if (pos == std::string::npos || pos + 3 >= test_case.size()) return -1.0;

    try {
        return std::stod(test_case.substr(pos + 3));
    } catch (...) {
        return -1.0;
    }
}

std::string extractGraphName(const std::string& test_case) {
    auto gp = test_case.find("graph_");
    if (gp == std::string::npos) return "unknown";

    auto us = test_case.find('_', gp + 6);
    return test_case.substr(gp + 6,
                            us == std::string::npos ? std::string::npos
                                                    : us - (gp + 6));
}

std::string timeCSVHeader() {
    return "test_case,graph_name,error_rate,algo,ec,n,m_init,m_updates,"
       "prediction_accuracy,case1_count,case2_count,"
       "online_total_us,online_avg_us_per_update,"
       "classical_total_us,classical_avg_us_per_update,"
       "speedup_vs_classical";
}

std::string etaCSVHeader() {
    return "test_case,graph_name,error_rate,algo,ec,step_j,last_match_i,eta_e,eta_v,eta_v_star";
}

Args parseArgs(int argc, char** argv) {
    Args a;
    for (int i = 1; i < argc; ++i) {
        std::string s(argv[i]);

        if (s == "--mode") {
            if (i + 1 >= argc) {
                std::cerr << "--mode requires a value\n";
                std::exit(1);
            }
            a.mode = argv[++i];
        } else if (s == "--runs") {
            if (i + 1 >= argc) {
                std::cerr << "--runs requires a value\n";
                std::exit(1);
            }
            a.runs = std::stoi(argv[++i]);
        } else if (s == "--csv-time") {
            if (i + 1 >= argc) {
                std::cerr << "--csv-time requires a value\n";
                std::exit(1);
            }
            a.csv_time = argv[++i];
        } else if (s == "--csv-eta") {
            if (i + 1 >= argc) {
                std::cerr << "--csv-eta requires a value\n";
                std::exit(1);
            }
            a.csv_eta = argv[++i];
        } else if (s == "--verify") {
            a.verify = true;
        } else if (s == "--ec") {
            a.errorCorrectionMode = ErrorCorrectionMode::NONTRIVIAL;
        }
        else if (s.rfind("--", 0) == 0) {
            std::cerr << "Unknown flag: " << s << "\n";
            std::exit(1);
        } else {
            a.input_file = s;
        }
    }

    if (a.input_file.empty()) {
        std::cerr << "Usage: ./benchmark --mode incremental|decremental|fullydynamic "
                     "[--runs N] [--csv-time file] [--csv-eta file] [--verify] testcase.txt\n";
        std::exit(1);
    }

    return a;
}

bool verifyResult(const QueryResult& r, const BFSState& real, int n) {
    BFSState check(n, real.source);
    check.outAdj = real.outAdj;
    check.inAdj = real.inAdj;
    check.computeBFS();

    for (int v = 1; v <= n; ++v) {
        if (r.level[v] != check.level[v]) {
            std::cerr << "[MISMATCH] step=" << r.step
                      << " v=" << v
                      << " got=" << r.level[v]
                      << " expected=" << check.level[v] << "\n";
            return false;
        }
    }
    return true;
}

void ensureCSVHeader(const std::string& path, const std::string& header) {
    if (!std::filesystem::exists(path)) {
        std::ofstream out(path);
        if (!out) {
            std::cerr << "Cannot open " << path << "\n";
            std::exit(1);
        }
        out << header << "\n";
    }
}

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
                          const AlgoTiming& classical) 
{
    const double speedup =
        (online.total_us > 1e-12) ? classical.total_us / online.total_us : 0.0;

    out << test_case << ","
        << graph_name << ","
        << ff(error_rate) << ","
        << algo << ","
        << ec << ","
        << n << ","
        << m_init << ","
        << m_updates << ","
        << ff(rm.prediction_accuracy) << ","
        << rm.case1_count << ","
        << rm.case2_count << ","
        << ff(online.total_us) << ","
        << ff(online.avg_us_per_update) << ","
        << ff(classical.total_us) << ","
        << ff(classical.per_update_avg_us) << ","
        << ff(speedup) << "\n";
}

void appendEtaRows(std::ofstream& out, const std::vector<EtaRow>& rows, const std::string& ec_label) {
    for (const auto& r : rows) {
        out << r.test_case << ","
            << r.graph_name << ","
            << ff(r.error_rate) << ","
            << r.algo << ","
            << ec_label << ","
            << r.step_j << ","
            << r.last_match_i << ","
            << r.eta_e << ","
            << ff(r.eta_v) << ","
            << ff(r.eta_v_star) << "\n";
    }
}

