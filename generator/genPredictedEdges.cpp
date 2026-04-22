#include <bits/stdc++.h>
using namespace std;
#ifdef CODER
#define dbg true
#include "debugger.h"
#else
#define dbg false
#define debug(x...)
#define debugptr(x,y)
#endif


using Update = tuple<int,int,int>;
void print(const vector<Update>& updates){
    for(auto& [u,v,type]: updates){
        cout << "(" << u << "," << v << "," << type << ") ";
    }
    cout << endl;
}

struct DynamicGraphInstance {//1 - indexing u and v
    int n;
    vector<pair<int,int>> initial_edges;
    vector<Update> realUpdates; // (u, v, type) : 0 insert, 1 delete
    vector<Update> predictedUpdates;
};

// read graph from file
DynamicGraphInstance read_instance(const string& filename) {
    ifstream in(filename);
    DynamicGraphInstance instance;

    in >> instance.n;
    int m0, m;
    in >> m0 >> m;

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
}

vector<Update> fixRealUpdates(const DynamicGraphInstance& instance){
    // if the edge is deleted but not inserted before, we will ignore the delete operation
    set<pair<int,int>> existing_edges;
    for(auto& e: instance.initial_edges){
        existing_edges.insert(e);
    }
    vector<Update> real_updates;
    for(auto& [u,v,type]: instance.realUpdates){
        if(type == 0){
            existing_edges.insert({u,v});
            real_updates.push_back({u,v,type});
        }else{
            if(existing_edges.count({u,v})){
                existing_edges.erase({u,v});
                real_updates.push_back({u,v,type});
            }
        }
    }
    return real_updates;
}

void shuffle_updates(vector<Update>& updates, int interval){
    assert(interval >= 0);
    if(interval == 0) return;

    interval = min(interval, (int)updates.size());
    // mt19937 rng(12345);
    mt19937 rng(std::random_device{}());
    for(int i = 0; i < (int)updates.size(); i += interval){
        int end = min(i + interval, (int)updates.size());
        shuffle(updates.begin() + i, updates.begin() + end, rng);
    }
}

void genIncrementalPredictedEdges(DynamicGraphInstance& instance, int interval){
    assert(interval >= 0);
    interval = min(interval, (int)instance.realUpdates.size());

    auto predUpdates = instance.realUpdates;
    shuffle_updates(predUpdates, interval);

    instance.predictedUpdates = std::move(predUpdates);
    // if(interval <= 100){
    //     print(instance.predictedUpdates);
    // }
}

void genDecrementalPredictedEdges(DynamicGraphInstance& instance, int interval){
    assert(interval >= 0);
    interval = min(interval, (int)instance.realUpdates.size());

    auto predUpdates = instance.realUpdates;
    shuffle_updates(predUpdates, interval);

    instance.predictedUpdates = std::move(predUpdates);
}

void genFullyDynamicPredictedEdges(DynamicGraphInstance& instance, int interval){
    assert(interval >= 0);
    interval = min(interval, (int)instance.realUpdates.size());

    instance.realUpdates = fixRealUpdates(instance);
    if(interval == 0){
        instance.predictedUpdates = instance.realUpdates;
        return;
    }

    auto predUpdates = instance.realUpdates;
    shuffle_updates(predUpdates, interval);

    // after shuffling, we will have some delete operations before the corresponding 
    // insert operations, we will move it to end to that interval
    set<pair<int,int>> existing_edges;
    for(auto& e: instance.initial_edges){
        existing_edges.insert(e);
    }

    for(int i = 0; i < (int)predUpdates.size(); i+= interval){
        std::vector<Update> remainingDeletedEdges;

        for (int j = i; j < min(i + interval, (int)predUpdates.size()); j++){
            auto& [u,v,type] = predUpdates[j];

            if(type == 0){
                existing_edges.insert({u,v});
                instance.predictedUpdates.push_back({u,v,type});
                continue;
            }
    
            if(existing_edges.count({u,v})){
                existing_edges.erase({u,v});
                instance.predictedUpdates.push_back({u,v,type});
                continue;
            }

            remainingDeletedEdges.push_back({u,v,type});
        }
        
        for (auto& update: remainingDeletedEdges){
            auto& [u,v,type] = update;
            instance.predictedUpdates.push_back({u,v,1}); 
            // it must be present in this interval or before this interval
        }
    }
}

int main() {
    // std::string str = "erdosRenyi";
    std::string str = "realGraphs";
    std::string folder = "../testcases/" + str + "/withoutPredictedEdges";
    // std::cout <<"Enter input folder name: " << std::endl;
    // std::cin >> folder;

    std::vector<std::string> files;

    for (const auto& entry : std::filesystem::directory_iterator(folder)) {
        files.push_back(entry.path().string());
    }

    std::cout << "Enter number of eta_e values: ";
    int k;
    std::cin >> k;

    std::vector<int> eta_es(k);
    std::cout << "Enter eta_e values (e.g. 0 5 10 20): ";
    for (int i = 0; i < k; i++) {
        std::cin >> eta_es[i];
    }

    int source = 1;

    debug(files);
    std::string out_folder = "../testcases/" + str + "/withPredictedEdges";

    for (const auto& eta_e : eta_es) {
        for (const auto& file : files) {
            debug(file);

            auto instance = read_instance(file);

            int interval = (eta_e * instance.realUpdates.size() + 99) / 100; // ceil
            if (eta_e == 0) interval = 0;
            else interval = std::max(2, interval);

            double eta_e_f = eta_e / 100.0;

            std::ostringstream oss;
            oss << std::fixed << std::setprecision(2) << eta_e_f;

            std::string out_file = out_folder + "/";

            if (file.find("mode_0") != std::string::npos) {
                genIncrementalPredictedEdges(instance, interval);
                out_file += "_mode_incremental_";
            } else if (file.find("mode_1") != std::string::npos) {
                genDecrementalPredictedEdges(instance, interval);
                out_file += "_mode_decremental_";
            } else if (file.find("mode_2") != std::string::npos) {
                genFullyDynamicPredictedEdges(instance, interval);
                out_file += "_mode_fullydynamic_";
            } else {
                assert(false);
            }

            out_file += oss.str() + "_" + std::filesystem::path(file).filename().string();

            std::ofstream out(out_file);
            out << "# Type: "
                << (file.find("mode_0") != std::string::npos
                        ? "incremental"
                        : (file.find("mode_1") != std::string::npos
                               ? "decremental"
                               : "fully dynamic"))
                << std::endl;
            out << "# Error rate: " << eta_e_f << std::endl;
            out << "# format: n source numInitialEdges numUpdates" << std::endl;
            out << "# next numInitialEdges lines: u v (directed edge u->v in G_0)" << std::endl;
            out << "# next numUpdates lines: u v type (real update)" << std::endl;
            out << "# next numUpdates lines: u v type (predicted update)" << std::endl;

            out << instance.n << ' ' << source << ' '
                << instance.initial_edges.size() << " "
                << instance.realUpdates.size() << std::endl;

            for (auto& [u, v] : instance.initial_edges) {
                out << u << " " << v << std::endl;
            }
            for (auto& [u, v, type] : instance.realUpdates) {
                out << u << " " << v << " " << type << std::endl;
            }
            for (auto& [u, v, type] : instance.predictedUpdates) {
                out << u << " " << v << " " << type << std::endl;
            }
        }
    }
}