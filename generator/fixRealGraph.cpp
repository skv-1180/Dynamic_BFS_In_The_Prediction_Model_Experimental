#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

struct Update {
    int u, v, type; // 0 = insert, 1 = delete
};

struct Edge {
    int u, v;

    bool operator==(const Edge& other) const {
        return u == other.u && v == other.v;
    }
};

struct EdgeHash {
    std::size_t operator()(const Edge& e) const {
        return (static_cast<std::size_t>(e.u) << 32) ^
               static_cast<std::size_t>(e.v);
    }
};

static bool isBlank(const std::string& s) {
    for (char c : s) {
        if (!std::isspace(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

// ------------------------------------------------------------
// Read full KONECT stream and sanitize globally:
// - ignore duplicate insertions
// - ignore invalid deletions
// ------------------------------------------------------------
std::vector<Update> readAndSanitizeKonekt(const std::string& inputFile) {
    std::ifstream fin(inputFile);
    if (!fin) {
        throw std::runtime_error("Cannot open input file: " + inputFile);
    }

    std::vector<Update> updates;
    std::unordered_set<Edge, EdgeHash> alive_edges;

    std::string line;
    while (std::getline(fin, line)) {
        if (line.empty() || isBlank(line)) continue;
        if (line[0] == '%') continue;

        std::istringstream iss(line);
        int u, v;
        std::string sign;

        if (!(iss >> u >> v >> sign)) continue;
        if (u <= 0 || v <= 0) continue;

        Edge e{u, v};

        if (sign == "+1") {
            if (alive_edges.find(e) != alive_edges.end()) continue;
            alive_edges.insert(e);
            updates.push_back({u, v, 0});
        } else if (sign == "-1") {
            auto it = alive_edges.find(e);
            if (it == alive_edges.end()) continue;
            alive_edges.erase(it);
            updates.push_back({u, v, 1});
        }
    }

    return updates;
}

// ------------------------------------------------------------
// Extract a smaller dynamic graph:
// - only keep edges with endpoints in [1..target_n]
// - keep stream order
// - enforce no duplicate insertions / invalid deletions inside subset
// - stop after target_m accepted updates
// ------------------------------------------------------------
std::vector<Update> buildSubsetGraph(
    const std::vector<Update>& fullUpdates,
    int target_n,
    int target_m)
{
    std::vector<Update> subset;
    subset.reserve(target_m);

    std::unordered_set<Edge, EdgeHash> alive_edges;

    for (const auto& up : fullUpdates) {
        if (up.u > target_n || up.v > target_n) continue;

        Edge e{up.u, up.v};

        if (up.type == 0) { // insert
            if (alive_edges.find(e) != alive_edges.end()) continue;
            alive_edges.insert(e);
            subset.push_back(up);
        } else { // delete
            auto it = alive_edges.find(e);
            if (it == alive_edges.end()) continue;
            alive_edges.erase(it);
            subset.push_back(up);
        }

        if ((int)subset.size() >= target_m) break;
    }

    return subset;
}

// ------------------------------------------------------------
// Write one output file in your required format
// ------------------------------------------------------------
void writeFormattedGraph(
    const std::string& outputFile,
    int n,
    const std::vector<Update>& updates)
{
    std::ofstream fout(outputFile);
    if (!fout) {
        throw std::runtime_error("Cannot open output file: " + outputFile);
    }

    fout << n << " " << 0 << " " << updates.size() << "\n";
    for (const auto& up : updates) {
        fout << up.u << " " << up.v << " " << up.type << "\n";
    }
}

// ------------------------------------------------------------
// Generate multiple graphs from one full stream
// specs = vector of {n, m}
// ------------------------------------------------------------
void generateMultipleGraphs(
    const std::string& inputFile,
    const std::vector<std::pair<int, int>>& specs,
    const std::string& outputPrefix)
{
    std::vector<Update> fullUpdates = readAndSanitizeKonekt(inputFile);

    for (const auto& [n, m] : specs) {
        std::vector<Update> subset = buildSubsetGraph(fullUpdates, n, m);

        std::string outputFile =
            outputPrefix + "_n" + std::to_string(n) +
            "_m" + std::to_string(m) + "_mode_2_.txt";

        writeFormattedGraph(outputFile, n, subset);

        std::cout << "Generated: " << outputFile
                  << "  with " << subset.size()
                  << " updates\n";
    }
}

// ------------------------------------------------------------
// Example main
// ------------------------------------------------------------
int main() {
    try {
        std::string inputFile = "konnect/raw.txt";

        std::vector<std::pair<int, int>> specs = {
            {100, 500},
            {200, 500},
            {500, 2000},
            {1000, 5000},
            {1000, 10000},
            {5000, 6000},
            {10000, 10000}
        };

        generateMultipleGraphs(inputFile, specs, "graph_subset");
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}