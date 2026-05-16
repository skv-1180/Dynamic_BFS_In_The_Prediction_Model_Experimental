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
 
 
static inline unsigned long long encodeEdge(int u, int v) {
    return (static_cast<unsigned long long>(u) << 32) ^ static_cast<unsigned int>(v);
}

inline void generateMoreEdge(std::vector<Update>& curr, int n, int m) {
    int m_curr = (int)curr.size();
    int req = m - m_curr;

    if (req <= 0) return;

    // m0 is zero always here.
    // So we reconstruct the current dynamic graph state only from curr.

    vector<pair<int, int>> activeEdges;
    vector<pair<int, int>> deletedEdges;

    unordered_map<unsigned long long, int> activePos;
    unordered_map<unsigned long long, int> deletedPos;

    auto removeFromActive = [&](int u, int v) {
        unsigned long long key = encodeEdge(u, v);

        auto it = activePos.find(key);
        if (it == activePos.end()) return;

        int idx = it->second;
        auto last = activeEdges.back();

        activeEdges[idx] = last;
        activeEdges.pop_back();

        activePos.erase(key);

        if (idx < (int)activeEdges.size()) {
            activePos[encodeEdge(last.first, last.second)] = idx;
        }
    };

    auto removeFromDeleted = [&](int u, int v) {
        unsigned long long key = encodeEdge(u, v);

        auto it = deletedPos.find(key);
        if (it == deletedPos.end()) return;

        int idx = it->second;
        auto last = deletedEdges.back();

        deletedEdges[idx] = last;
        deletedEdges.pop_back();

        deletedPos.erase(key);

        if (idx < (int)deletedEdges.size()) {
            deletedPos[encodeEdge(last.first, last.second)] = idx;
        }
    };

    auto addToActive = [&](int u, int v) {
        if (u == v) return;

        unsigned long long key = encodeEdge(u, v);

        if (activePos.count(key)) return;

        if (deletedPos.count(key)) {
            removeFromDeleted(u, v);
        }

        activePos[key] = (int)activeEdges.size();
        activeEdges.push_back({u, v});
    };

    auto addToDeleted = [&](int u, int v) {
        if (u == v) return;

        unsigned long long key = encodeEdge(u, v);

        if (deletedPos.count(key)) return;

        if (activePos.count(key)) {
            removeFromActive(u, v);
        }

        deletedPos[key] = (int)deletedEdges.size();
        deletedEdges.push_back({u, v});
    };

    // Replay existing dynamic updates.
    for (const auto& e : curr) {
        int u = e.u;
        int v = e.v;
        int type = e.type;

        if (u < 1 || u > n || v < 1 || v > n || u == v) continue;

        if (type == 0) {
            // insertion
            addToActive(u, v);
        } else {
            // deletion
            addToDeleted(u, v);
        }
    }

    mt19937 rng(
        chrono::steady_clock::now().time_since_epoch().count()
    );

    auto deleteRandomActive = [&]() -> Update {
        uniform_int_distribution<int> dist(0, (int)activeEdges.size() - 1);
        int idx = dist(rng);

        auto [u, v] = activeEdges[idx];

        removeFromActive(u, v);

        unsigned long long key = encodeEdge(u, v);
        deletedPos[key] = (int)deletedEdges.size();
        deletedEdges.push_back({u, v});

        return {u, v, 1};
    };

    auto insertRandomDeleted = [&]() -> Update {
        uniform_int_distribution<int> dist(0, (int)deletedEdges.size() - 1);
        int idx = dist(rng);

        auto [u, v] = deletedEdges[idx];

        removeFromDeleted(u, v);

        unsigned long long key = encodeEdge(u, v);
        activePos[key] = (int)activeEdges.size();
        activeEdges.push_back({u, v});

        return {u, v, 0};
    };

    while ((int)curr.size() < m) {
        bool canDelete = !activeEdges.empty();
        bool canInsert = !deletedEdges.empty();

        if (!canDelete && !canInsert) {
            break;
        }

        int op;

        if (canDelete && canInsert) {
            uniform_int_distribution<int> opDist(0, 1);
            op = opDist(rng); 
            // op = 0 insertion
            // op = 1 deletion
        } else if (canDelete) {
            op = 1;
        } else {
            op = 0;
        }

        // Random batch size: this gives patterns like
        // delete delete insert insert insert delete ...
        uniform_int_distribution<int> batchDist(1, 4);
        int batchSize = batchDist(rng);

        for (int i = 0; i < batchSize && (int)curr.size() < m; i++) {
            if (op == 1) {
                if (activeEdges.empty()) break;

                Update e = deleteRandomActive();
                curr.push_back(e);
            } else {
                if (deletedEdges.empty()) break;

                Update e = insertRandomDeleted();
                curr.push_back(e);
            }
        }
    }
}