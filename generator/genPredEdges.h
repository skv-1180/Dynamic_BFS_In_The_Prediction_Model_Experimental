#pragma once
#include <bits/stdc++.h>
using namespace std;


using Update = tuple<int, int, int>;  // (u, v, type) : 0 insert, 1 delete
std::ostream& operator<<(std::ostream& out,  const Update& up){
    const auto& [u, v, t] = up;
    out << "[" << u << ' ' << v << ' ' << t << "]";
    return out;
}

#include "debugger.h"

// Basic helpers

struct Edge {
    int u, v;
    bool operator==(const Edge& other) const { return u == other.u && v == other.v; }
    bool operator<(const Edge& other) const {
        if (u != other.u) return u < other.u;
        return v < other.v;
    }
};

struct EdgeHash {
    size_t operator()(const Edge& e) const {
        return (static_cast<size_t>(e.u) << 32) ^ static_cast<size_t>(e.v);
    }
};

using EdgeSet = unordered_set<Edge, EdgeHash>;

static Edge toEdge(const Update& up) {
    auto [u, v, type] = up;
    (void)type;
    return {u, v};
}

static bool sameUpdate(const Update& a, const Update& b) {
    return a == b;
}

static bool isCommentOrBlank(const string& s) {
    for (char c : s) {
        if (!isspace(static_cast<unsigned char>(c))) {
            return c == '#';
        }
    }
    return true;
}

static void printUpdates(const vector<Update>& updates) {
    for (auto& [u, v, t] : updates) {
        cout << "(" << u << "," << v << "," << t << ") ";
    }
    cout << "\n";
}

// Instance

struct DynamicGraphInstance {
    int n = 0;
    int source = 1;
    vector<pair<int, int>> initial_edges;
    vector<Update> realUpdates;
    vector<Update> predictedUpdates;
};

// Supports both:
//   n m0 m
// and
//   n source m0 m
DynamicGraphInstance read_instance(const string& filename) {
    ifstream in(filename);
    if (!in) {
        throw runtime_error("Cannot open file: " + filename);
    }

    DynamicGraphInstance instance;

    // Skip comments
    string line;
    while (getline(in, line)) {
        if (line.empty() || isCommentOrBlank(line)) continue;
        stringstream ss(line);
        vector<int> vals;
        int x;
        while (ss >> x) vals.push_back(x);

        if (vals.size() == 3) {
            instance.n = vals[0];
            int m0 = vals[1];
            int m = vals[2];

            instance.initial_edges.resize(m0);
            for (int i = 0; i < m0; i++) {
                int u, v;
                in >> u >> v;
                instance.initial_edges[i] = {u, v};
            }

            instance.realUpdates.resize(m);
            for (int i = 0; i < m; i++) {
                int u, v, type;
                in >> u >> v >> type;
                instance.realUpdates[i] = {u, v, type};
            }
            return instance;
        } else if (vals.size() == 4) {
            instance.n = vals[0];
            instance.source = vals[1];
            int m0 = vals[2];
            int m = vals[3];

            instance.initial_edges.resize(m0);
            for (int i = 0; i < m0; i++) {
                int u, v;
                in >> u >> v;
                instance.initial_edges[i] = {u, v};
            }

            instance.realUpdates.resize(m);
            for (int i = 0; i < m; i++) {
                int u, v, type;
                in >> u >> v >> type;
                instance.realUpdates[i] = {u, v, type};
            }
            return instance;
        } else {
            throw runtime_error("Bad header format in file: " + filename);
        }
    }

    throw runtime_error("Empty input file: " + filename);
}

// Sanitize real updates
// - no duplicate insertions
// - no invalid deletions

vector<Update> sanitizeRealUpdates(const DynamicGraphInstance& instance) {
    EdgeSet alive;
    for (auto [u, v] : instance.initial_edges) alive.insert({u, v});

    vector<Update> cleaned;
    cleaned.reserve(instance.realUpdates.size());

    for (auto [u, v, type] : instance.realUpdates) {
        Edge e{u, v};
        if (type == 0) {
            if (alive.find(e) == alive.end()) {
                alive.insert(e);
                cleaned.push_back({u, v, 0});
            }
        } else {
            auto it = alive.find(e);
            if (it != alive.end()) {
                alive.erase(it);
                cleaned.push_back({u, v, 1});
            }
        }
    }
    return cleaned;
}

// Apply update to an edge set
// Assumes update is valid for the current set.

static void applyUpdate(EdgeSet& S, const Update& up) {
    auto [u, v, type] = up;
    Edge e{u, v};

    if (type == 0) {
        S.insert(e);
    } else {
        auto it = S.find(e);
        if (it != S.end()) S.erase(it);
    }
}

static void applyUpdate(set<Edge>& S, const Update& up) {
    auto [u, v, type] = up;
    Edge e{u, v};

    if (type == 0) {
        S.insert(e);
    } else {
        auto it = S.find(e);
        if (it != S.end()) S.erase(it);
    }
}

// Build arr[0..m-1]
// arr[i] = 1 means state match after update i
// We force arr[m-1] = 1

vector<int> buildMatchArray(int m, double error_rate, mt19937& rng) {
    vector<int> arr(m, 0);
    bernoulli_distribution matchDist(1.0 - error_rate);

    for (int i = 0; i < m; ++i) {
        arr[i] = matchDist(rng) ? 1 : 0;
    }
    if (m > 0) arr[m - 1] = 1;
    return arr;
}

// Block generation
//
// For a block [L..R] with the invariant:
//   real state == predicted state before L
// and we want:
//   real state == predicted state after R
//
// We generate a new interleaving of the same block updates,
// while preserving per-edge order.

// Candidate interleaving preserving per-edge order.
// Tries to differ from the real prefix when possible.
vector<Update> makeOneInterleavingPreservePerEdgeOrder(
    const vector<Update>& realBlock,
    mt19937& rng)
{
    int len = (int)realBlock.size();
    if (len <= 1) return realBlock;

    // Group by edge, preserving per-edge order.
    map<Edge, vector<Update>> edgeSeq;
    vector<Edge> edgeOrder;
    set<Edge> seen;

    for (const auto& up : realBlock) {
        Edge e = toEdge(up);
        if (!seen.count(e)) {
            seen.insert(e);
            edgeOrder.push_back(e);
        }
        edgeSeq[e].push_back(up);
    }

    map<Edge, int> ptr;
    for (const auto& e : edgeOrder) ptr[e] = 0;

    vector<Update> out;
    out.reserve(len);

    for (int pos = 0; pos < len; ++pos) {
        vector<Edge> available;
        vector<Edge> preferred;

        for (const auto& e : edgeOrder) {
            if (ptr[e] < (int)edgeSeq[e].size()) {
                available.push_back(e);
                const Update& cand = edgeSeq[e][ptr[e]];
                if (!sameUpdate(cand, realBlock[pos])) {
                    preferred.push_back(e);
                }
            }
        }

        vector<Edge>& choicePool = preferred.empty() ? available : preferred;
        uniform_int_distribution<int> dist(0, (int)choicePool.size() - 1);
        Edge chosen = choicePool[dist(rng)];

        out.push_back(edgeSeq[chosen][ptr[chosen]]);
        ptr[chosen]++;
    }

    return out;
}


// Count how many internal steps are mismatched if we use predBlock
// against realBlock, starting from the same startState.
// We do NOT count the last position, since the block endpoint must match.
int countInternalMismatches(
    const EdgeSet& startState,
    const vector<Update>& realBlock,
    const vector<Update>& predBlock)
{
    EdgeSet realState = startState;
    EdgeSet predState = startState;

    int len = (int)realBlock.size();
    int bad = 0;

    for (int i = 0; i < len; ++i) {
        applyUpdate(realState, realBlock[i]);
        applyUpdate(predState, predBlock[i]);

        if (i != len - 1 && realState != predState) {
            bad++;
        }
    }

    return bad;
}

// Best-of-many trials interleaving.
vector<Update> buildPredictedBlock(
    const EdgeSet& startState,
    const vector<Update>& realBlock,
    mt19937& rng,
    int trials = 1)
{
    int len = (int)realBlock.size();
    if (len <= 1) return realBlock;

    vector<Update> best = realBlock;
    int bestScore = -1;

    vector<Update> cand = makeOneInterleavingPreservePerEdgeOrder(realBlock, rng);
    best = std::move(cand);

    return best;
}

// Shuffle updates while preserving order of updates for the same edge.
// Expected O(m) time.
// For the same (u, v), the relative order of types is unchanged.
vector<Update> shuffleUpdatesPreservePerEdgeOrder(
    const vector<Update>& updates,
    mt19937& rng)
{
    int m = (int)updates.size();
    if (m <= 1) return updates;

    // For each edge, store its updates in original order.
    unordered_map<Edge, vector<Update>, EdgeHash> edgeSeq;
    edgeSeq.reserve(m * 2);

    vector<Edge> edgeOrder;
    edgeOrder.reserve(m);

    for (const auto& up : updates) {
        Edge e = toEdge(up);

        auto it = edgeSeq.find(e);
        if (it == edgeSeq.end()) {
            edgeSeq.emplace(e, vector<Update>{});
            edgeOrder.push_back(e);
        }

        edgeSeq[e].push_back(up);
    }

    // Pointer for next unused update of each edge.
    unordered_map<Edge, int, EdgeHash> ptr;
    ptr.reserve(edgeSeq.size() * 2);

    // Active edges are edges that still have remaining updates.
    vector<Edge> active;
    active.reserve(edgeOrder.size());

    for (const Edge& e : edgeOrder) {
        ptr[e] = 0;
        active.push_back(e);
    }

    vector<Update> shuffled;
    shuffled.reserve(m);

    while (!active.empty()) {
        uniform_int_distribution<int> dist(0, (int)active.size() - 1);
        int idx = dist(rng);

        Edge e = active[idx];
        int p = ptr[e];

        // Consume next update of this edge.
        shuffled.push_back(edgeSeq[e][p]);
        ptr[e]++;

        // If this edge is exhausted, remove it from active in O(1).
        if (ptr[e] == (int)edgeSeq[e].size()) {
            active[idx] = active.back();
            active.pop_back();
        }
    }

    return shuffled;
}

void generatePredictedByStateError(
    DynamicGraphInstance& instance,
    double error_rate,
    uint32_t seed = 123456789)
{
    mt19937 rng(seed);

    // Clean real updates first.
    instance.realUpdates = sanitizeRealUpdates(instance);

    int m = (int)instance.realUpdates.size();
    instance.predictedUpdates.clear();
    instance.predictedUpdates.reserve(m);

    if (m == 0) return;

    if (error_rate == 1.0) {
        instance.predictedUpdates =
            shuffleUpdatesPreservePerEdgeOrder(instance.realUpdates, rng);
        return;
    }

    vector<int> arr = buildMatchArray(m, error_rate, rng);
    // debug(arr)

    // State before current block
    EdgeSet matchedState;
    for (auto [u, v] : instance.initial_edges) matchedState.insert({u, v});
    
    int lastMatchIdx = -1;

    for (int i = 0; i < m; ++i) {
        if (arr[i] == 1) {
            int L = lastMatchIdx + 1;
            int R = i;

            vector<Update> realBlock(
                instance.realUpdates.begin() + L,
                instance.realUpdates.begin() + R + 1
            );

            vector<Update> predBlock = buildPredictedBlock(matchedState, realBlock, rng);

            // Append predicted block
            for (const auto& up : predBlock) {
                instance.predictedUpdates.push_back(up);
            }

            // Advance matchedState by real block
            // (same final state as pred block by construction)
            for (const auto& up : realBlock) {
                applyUpdate(matchedState, up);
            }

            lastMatchIdx = i;
        }
    }

    // Since arr[m-1] = 1, everything should be covered.
    if ((int)instance.predictedUpdates.size() != m) {
        throw runtime_error("Predicted update sequence size mismatch");
    }
}

// Diagnostics
/*
using EdgeSet = unordered_set<Edge, EdgeHash>;

*/

double computeAchievedStateErrorRate(const DynamicGraphInstance& instance) {
    // EdgeSet realState, predState;
    set<Edge> realState, predState;
    for (auto [u, v] : instance.initial_edges) {
        realState.insert({u, v});
        predState.insert({u, v});
    }

    int m = (int)instance.realUpdates.size();
    if ((int)instance.predictedUpdates.size() != m) {
        throw runtime_error("real/pred size mismatch in achieved-error computation");
    }

    int mismatchCount = 0;
    for (int i = 0; i < m; ++i) {
        applyUpdate(realState, instance.realUpdates[i]);
        applyUpdate(predState, instance.predictedUpdates[i]);
        if (realState != predState) mismatchCount++;
    }

    // debug(mismatchCount, m)

    return (m == 0 ? 0.0 : (double)mismatchCount / m);
}

// Write output in your benchmark format

void write_instance(const string& out_file, const DynamicGraphInstance& instance, double nominal_error_rate) {
    ofstream out(out_file);
    if (!out) {
        throw runtime_error("Cannot open output file: " + out_file);
    }

    out << "# Nominal state-based error rate: " << fixed << setprecision(2) << nominal_error_rate << "\n";
    // out << "# Achieved state-based error rate: "
        // << fixed << setprecision(4) << computeAchievedStateErrorRate(instance) << "\n";
    out << "# format: n source numInitialEdges numUpdates\n";
    out << "# next numInitialEdges lines: u v (directed edge u->v in G_0)\n";
    out << "# next numUpdates lines: u v type (real update)\n";
    out << "# next numUpdates lines: u v type (predicted update)\n";

    out << instance.n << " " << instance.source << " "
        << instance.initial_edges.size() << " "
        << instance.realUpdates.size() << "\n";

    for (auto [u, v] : instance.initial_edges) {
        out << u << " " << v << "\n";
    }
    for (auto [u, v, type] : instance.realUpdates) {
        out << u << " " << v << " " << type << "\n";
    }
    for (auto [u, v, type] : instance.predictedUpdates) {
        out << u << " " << v << " " << type << "\n";
    }
}

// ----------------
void generatePredictedByPositionError(
    DynamicGraphInstance& instance,
    double error_rate,
    uint32_t seed = 123456789)
{
    std::mt19937 rng(seed);

    // Clean real updates first.
    instance.realUpdates = sanitizeRealUpdates(instance);

    int m = (int)instance.realUpdates.size();
    instance.predictedUpdates.clear();
    instance.predictedUpdates.resize(m);

    if (m == 0) return;

    // Clamp error_rate to [0, 1]
    error_rate = std::max(0.0, std::min(1.0, error_rate));

    int correct_cnt = (int)std::round((1.0 - error_rate) * m);

    // indices = [0, 1, 2, ..., m-1]
    std::vector<int> indices(m);
    std::iota(indices.begin(), indices.end(), 0);

    // Randomly choose positions that will remain correct
    std::shuffle(indices.begin(), indices.end(), rng);

    std::vector<char> fixed(m, false);

    for (int i = 0; i < correct_cnt; ++i) {
        int idx = indices[i];
        fixed[idx] = true;
        instance.predictedUpdates[idx] = instance.realUpdates[idx];
    }

    // Collect remaining positions and their updates
    std::vector<int> free_pos;
    std::vector<Update> free_updates;

    for (int i = 0; i < m; ++i) {
        if (!fixed[i]) {
            free_pos.push_back(i);
            free_updates.push_back(instance.realUpdates[i]);
        }
    }

    // Shuffle only the remaining updates
    std::shuffle(free_updates.begin(), free_updates.end(), rng);

    // Fill shuffled updates into non-fixed positions
    for (int j = 0; j < (int)free_pos.size(); ++j) {
        int pos = free_pos[j];
        instance.predictedUpdates[pos] = free_updates[j];
    }

    if ((int)instance.predictedUpdates.size() != m) {
        throw std::runtime_error("Predicted update sequence size mismatch");
    }
}