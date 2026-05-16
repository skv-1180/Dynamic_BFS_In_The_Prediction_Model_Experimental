#include <bits/stdc++.h>
using namespace std;
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

        if (!(iss >> u >> v)) continue;
        if (u <= 0 || v <= 0) continue;

        Edge e{u, v};

        if (alive_edges.find(e) != alive_edges.end()) continue;
        alive_edges.insert(e);
        updates.push_back({u, v});
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

        if (alive_edges.find(e) != alive_edges.end()) continue;
        alive_edges.insert(e);
        subset.push_back(up);

        if ((int)subset.size() >= (target_m)) break;
    }
    // assert(target_m <= (target_n*(target_n+1)/2));

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> dist(1, target_n);

  
    while ((int)subset.size() < target_m) {
        int u = dist(gen);
        int v = dist(gen);

        Edge e{u, v};

        if(u == v) continue;
        if (alive_edges.find(e) != alive_edges.end()) {
            continue;
        }

        alive_edges.insert(e);

        subset.push_back(Update{
            u,
            v
        });
    }
/*
std::vector<Edge> candidates;
candidates.reserve(target_n * (target_n - 1));

for (int u = 1; u <= target_n; ++u) {
    for (int v = 1; v <= target_n; ++v) {
        if (u == v) continue;

        Edge e{u, v};

        if (alive_edges.find(e) == alive_edges.end()) {
            candidates.push_back(e);
        }
    }
}

std::random_device rd;
std::mt19937 gen(rd());

std::shuffle(candidates.begin(), candidates.end(), gen);

size_t need = target_m - subset.size();

assert(need <= candidates.size());

for (size_t i = 0; i < need; ++i) {
    const Edge& e = candidates[i];

    alive_edges.insert(e);

    subset.push_back(Update{
        e.u,
        e.v
    });
}
    */

    return subset;
}

// ------------------------------------------------------------
// Write one output file in your required format
// ------------------------------------------------------------
void writeFormattedGraph(
    const std::string& outputFile,
    int n,
    const std::vector<Update>& updates, int mode)
{
    std::ofstream fout(outputFile);
    if (!fout) {
        throw std::runtime_error("Cannot open output file: " + outputFile);
    }

    int m = updates.size() ;
    if(mode == 0){
        fout << n << " " << 0 << " " << m << "\n";
        for (const auto& up : updates) {
            fout << up.u << " " << up.v << " " << 0 << "\n";
        }
    }else{
        int m0 = (m+1)/2;
        int m1 = m - m0;
        fout << n << " " << m0 << " " << m1 << "\n";
        for(int i=0;i<m0;i++){
            auto up = updates[i];
            fout << up.u << " " << up.v << "\n";
        }

        for(int i=m1-1;i>=0;i--){
            auto up = updates[i];
            fout << up.u << " " << up.v << " " << 1 << "\n";
        }
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
        
        std::string outputFile1 = outputPrefix + "_n" + std::to_string(n) +
            "_m" + std::to_string(subset.size()) + "_mode_0_.txt";
        std::string outputFile2 = outputPrefix + "_n" + std::to_string(n) +
            "_m" + std::to_string(subset.size()) + "_mode_1_.txt";

        writeFormattedGraph(outputFile1, n, subset, 0);
        writeFormattedGraph(outputFile2, n, subset, 1);

        std::cout << "Generated: " << outputFile1
                  << "  with " << subset.size()
                  << " updates\n";

        std::cout << "Generated: " << outputFile2
                  << "  with " << subset.size()
                  << " updates\n";
    }
}

// ------------------------------------------------------------
// Example main
// ------------------------------------------------------------
int main() {
    try {
        std::string inputFile = "incdec/konnect/rawInc.txt";

        std::vector<std::pair<int, int>> specs = {
            // {1000,1000000},
            // {1000,900000},
            // {500,250000},
            // {500,200000},
            // {1000,100000},
            // {500,100000},
            // {300, 90000},

            // {1000,30000},
            // {1000,50000},
            {1000,100000},
            
            // {5000, 50000},
            // {5000, 100000},
            // {10000, 100000},
            // {10000, 200000},
            // {10000, 20000},
            // {10000, 30000},
            // {10000, 40000},
            // {10000, 50000},
            // {10000, 100000},
            // {10000, 100000},
            // {10000, 150000},
            // {10000, 200000},

            // {50000, 100000},
            // {50000, 150000},
            // {50000, 200000},
        };

        generateMultipleGraphs(inputFile, specs, "graph_subset");
    } catch (const std::exception& ex) {
        std::cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}