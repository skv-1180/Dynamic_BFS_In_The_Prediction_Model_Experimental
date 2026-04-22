#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/erdos_renyi_generator.hpp>
#include <boost/random/mersenne_twister.hpp>
#include <boost/graph/graph_traits.hpp>

#include <iostream>
#include <vector>
#include <set>
#include <tuple>
#include <random>
#include <algorithm>
#include <stdexcept>

using namespace std;
#ifdef CODER
#define dbg true
#include "debugger.h"
#else
#define dbg false
#define debug(x...)
#define debugptr(x,y)
#endif

using Graph = boost::adjacency_list<
    boost::setS,
    boost::vecS,
    boost::directedS
>;

using ERGen = boost::erdos_renyi_iterator<boost::mt19937, Graph>;
using EdgeIt = boost::graph_traits<Graph>::edge_iterator;

struct DynamicGraphInstance {
    int n;
    vector<pair<int,int>> initial_edges;
    vector<tuple<int,int,int>> updates; // (u, v, type) : 0 insert, 1 delete
};

static inline long long max_directed_edges_no_self_loop(int n) {
    return 1LL * n * (n - 1);
}

static pair<int,int> random_non_edge(
    int n,
    set<pair<int,int>>& edge_set,
    mt19937& rng
) {
    uniform_int_distribution<int> dist(0, n - 1);

    while (true) {
        int u = dist(rng);
        int v = dist(rng);
        if (u == v) continue;
        if (!edge_set.count({u, v})) {
            return {u, v};
        }
    }
}

static pair<int,int> random_existing_edge(
    const vector<pair<int,int>>& edges,
    mt19937& rng
) {
    uniform_int_distribution<int> dist(0, (int)edges.size() - 1);
    return edges[dist(rng)];
}

static void rebuild_edge_vector_from_set(
    const set<pair<int,int>>& edge_set,
    vector<pair<int,int>>& edges
) {
    edges.clear();
    for (auto &e : edge_set) edges.push_back(e);
}

vector<pair<int,int>> add_root_edges_from_zero(
    const vector<pair<int,int>>& edge_list,
    int n
) {
    vector<int> indegree(n, 0);

    for (auto &e : edge_list) {
        indegree[e.second]++;
    }

    vector<pair<int,int>> new_edges = edge_list;

    for (int v = 0; v < n; v++) {
        if (v == 0) continue;

        if (indegree[v] == 0) {
            new_edges.push_back({0, v});
        }
    }

    return new_edges;
}

vector<pair<int,int>> generate_random_directed_graph_edges(int n, int p, uint32_t seed = 42) {
    if (n <= 0) {
        throw invalid_argument("n must be positive");
    }
    if (p < 0 || 1LL * p > max_directed_edges_no_self_loop(n)) {
        throw invalid_argument("p is invalid for directed graph without self-loops");
    }

    boost::mt19937 boost_rng(seed);

    Graph g(
        ERGen(
            boost_rng,
            static_cast<Graph::vertices_size_type>(n),
            static_cast<Graph::edges_size_type>(p),
            false
        ),
        ERGen(),
        static_cast<Graph::vertices_size_type>(n)
    );
    debug(boost::num_edges(g))
    vector<pair<int,int>> edge_list;
    edge_list.reserve(boost::num_edges(g));

    EdgeIt ei, ei_end;
    for (boost::tie(ei, ei_end) = edges(g); ei != ei_end; ++ei) {
        int u = (int)source(*ei, g);
        int v = (int)target(*ei, g);
        edge_list.push_back({u, v});
    }
    edge_list = add_root_edges_from_zero(edge_list, n);
    debug(p)
    debug(edge_list)
    return edge_list;
}

// Incremental: generate graph with (m0 + m) edges, keep first m0 as initial, remaining m as insertions
DynamicGraphInstance generate_incremental(int n, int m0, int m, uint32_t seed = 42) {
    if (m0 < 0 || m < 0) {
        throw invalid_argument("m0 and m must be non-negative");
    }
    if (1LL * (m0 + m) > max_directed_edges_no_self_loop(n)) {
        throw invalid_argument("m0 + m exceeds max possible directed edges");
    }

    auto all_edges = generate_random_directed_graph_edges(n, m0 + m, seed);

    if ((int)all_edges.size() < m0 + m) {
        int generated = (int)all_edges.size();
        if(m0 > generated) {
            throw runtime_error("Generated fewer edges than expected for initial graph. Try a different seed.");
        } else {
            m = generated - m0; // adjust m to the number of edges we can actually insert
        }
    }

    DynamicGraphInstance result;
    result.n = n;

    for (int i = 0; i < m0; i++) {
        result.initial_edges.push_back(all_edges[i]);
    }

    for (int i = m0; i < m0 + m; i++) {
        auto [u, v] = all_edges[i];
        result.updates.push_back({u, v, 0});
    }

    return result;
}

// Decremental: generate graph with m0 edges, choose m of those as deletions
DynamicGraphInstance generate_decremental(int n, int m0, int m, uint32_t seed = 42) {
    if (m0 < 0 || m < 0) {
        throw invalid_argument("m0 and m must be non-negative");
    }
    if (m > m0) {
        throw invalid_argument("For decremental, m cannot exceed m0");
    }

    auto initial_edges = generate_random_directed_graph_edges(n, m0, seed);
    m0 = (int)initial_edges.size(); // adjust m0 to actual generated edges if needed
    if(m0 < m) {
        throw runtime_error("Generated fewer edges than expected for initial graph. Try a different seed.");
    }

    DynamicGraphInstance result;
    result.n = n;
    result.initial_edges = initial_edges;

    mt19937 rng(seed + 100);
    vector<int> idx(m0);
    for (int i = 0; i < m0; i++) idx[i] = i;
    shuffle(idx.begin(), idx.end(), rng);

    for (int i = 0; i < m; i++) {
        auto [u, v] = initial_edges[idx[i]];
        result.updates.push_back({u, v, 1});
    }

    return result;
}

struct EdgeHash {
    size_t operator()(const pair<int,int>& p) const {
        return ((size_t)p.first << 32) ^ p.second;
    }
};

struct FastEdgeSet {
    vector<pair<int,int>> edges;
    unordered_map<pair<int,int>, int, EdgeHash> pos;

    bool exists(int u, int v) {
        return pos.count({u,v});
    }

    void insert(int u, int v) {
        if (exists(u,v)) return;
        pos[{u,v}] = edges.size();
        edges.push_back({u,v});
    }

    void erase(int u, int v) {
        auto it = pos.find({u,v});
        if (it == pos.end()) return;

        int idx = it->second;
        auto last = edges.back();

        // swap with last
        edges[idx] = last;
        pos[last] = idx;

        edges.pop_back();
        pos.erase(it);
    }

    pair<int,int> get_random(mt19937& rng) {
        uniform_int_distribution<int> dist(0, edges.size() - 1);
        return edges[dist(rng)];
    }
};

// Fully Dynamic: start with m0 initial edges - do mixed insert/delete
// after every 2-3 insertions, try a deletion from currently existing edges
DynamicGraphInstance generate_fully_dynamic(int n, int m0, int m, uint32_t seed = 42) {
    auto initial_edges = generate_random_directed_graph_edges(n, m0, seed);

    DynamicGraphInstance result;
    result.n = n;
    result.initial_edges = initial_edges;

    mt19937 rng(seed + 200);

    FastEdgeSet edgeSet;

    for (auto &e : initial_edges) {
        edgeSet.insert(e.first, e.second);
    }

    uniform_int_distribution<int> node_dist(0, n - 1);

    int updates_done = 0;
    int insert_count = 0;

    while (updates_done < m) {
        bool do_delete = false;

        if (!edgeSet.edges.empty() && insert_count >= 2) {
            do_delete = (rng() % 2); // ~50% chance
        }

        if (do_delete) {
            auto [u,v] = edgeSet.get_random(rng);

            result.updates.push_back({u, v, 1});
            edgeSet.erase(u, v);

            insert_count = 0;
        } else {
            int u, v;
            do {
                u = node_dist(rng);
                v = node_dist(rng);
            } while (u == v || edgeSet.exists(u, v));

            result.updates.push_back({u, v, 0});
            edgeSet.insert(u, v);

            insert_count++;
        }

        updates_done++;
    }

    return result;
}

void print_instance(const DynamicGraphInstance& instance, std::ostream& out = std::cout) {
    out << instance.n << " " << instance.initial_edges.size() << " " << instance.updates.size() << "\n";

    for (auto &[u, v] : instance.initial_edges) {
        out << u + 1 << " " << v + 1 << "\n";
    }

    for (auto &[u, v, type] : instance.updates) {
        out << u + 1 << " " << v + 1 << " " << type << "\n";
    }
}

/*
int main() {
    int n, m0, m;
    int mode; // 0 incremental, 1 decremental, 2 fully dynamic
    #ifdef CODER
    std::cout << "Enter n, m0, m, mode(0 incremental, 1 decremental, 2 fully dynamic): ";
    #endif

    std::cin >> n >> m0 >> m >> mode;
    uint32_t seed = 42;

    DynamicGraphInstance instance;
    if(mode == 0) {
        instance = generate_incremental(n, m0, m, seed);
    } else if(mode == 1) {
        instance = generate_decremental(n, m0, m, seed);
    } else if(mode == 2) {
        instance = generate_fully_dynamic(n, m0, m, seed);
    } else {
        cerr << "Invalid mode\n";
    }
    #ifdef CODER
    std::cout << "update m0 and m: " << instance.initial_edges.size() << " " << instance.updates.size() << "\n";
    #endif

    print_instance(instance);
    return 0;
}
*/