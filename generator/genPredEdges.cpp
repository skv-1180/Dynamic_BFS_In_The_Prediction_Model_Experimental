#include "genPredEdges.h"
using namespace std;
/*
21
0 0.05 0.1 0.15 0.2 0.25 0.3 0.35 0.4 0.45 0.5 0.55 0.6 0.65 0.7 0.75 0.8 0.85 0.9 0.95 1.0
0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1.0
*/

int main() {
    try {
        // string folder = "testcases/erdos/withoutPredictedEdges";
        // string folder = "testcases/realGraphs/withoutLarge";
        // string out_folder = "testcases/withPredictedEdgesReal";
        
        // standford
        string folder = "../data/benchmark/testcases";
        string out_folder = "testcases/withPredictedEdgesRealStandFord";

        vector<string> files;
        for (const auto& entry : filesystem::directory_iterator(folder)) {
            files.push_back(entry.path().string());
        }
        sort(files.begin(), files.end());

        cout << "Enter number of error-rate values: ";
        int k;
        cin >> k;

        vector<double> error_rates(k);
        cout << "Enter error rates (e.g. 0 0.05 0.10 0.20 0.50): ";
        for (int i = 0; i < k; i++) cin >> error_rates[i];

        filesystem::create_directories(out_folder);

        uint32_t base_seed = 
        std::random_device{}() ^
        std::chrono::high_resolution_clock::now().time_since_epoch().count();

        for (double p : error_rates) {
            for (int fi = 0; fi < (int)files.size(); ++fi) {
                auto instance = read_instance(files[fi]);

                generatePredictedByStateError(instance, p, base_seed + fi * 1000 + (int)round(p * 1000));

                if((instance.initial_edges.size() + instance.realUpdates.size()) <= 10000){
                    double achievedErrorRate = computeAchievedStateErrorRate(instance);
                    p = achievedErrorRate;
                }

                ostringstream oss;
                oss << fixed << setprecision(2) << p;

                string out_file = out_folder + "/graph_";
                string tmp = files[fi];
                if ((tmp.find("mode_0") != std::string::npos) || (tmp.find("mode_incremental") != std::string::npos)) {
                    out_file += "mode_incremental_";
                } else if ((tmp.find("mode_1") != std::string::npos) || (tmp.find("mode_decremental") != std::string::npos)) {
                    out_file += "mode_decremental_";
                } else if ((tmp.find("mode_2") != std::string::npos) || (tmp.find("mode_fullydynamic") != std::string::npos)) {
                    out_file += "mode_fullydynamic_";
                } else {
                    assert(false);
                }

                out_file += "err" + oss.str() + "_" +
                                  filesystem::path(files[fi]).filename().string();

                write_instance(out_file, instance, p);

                cout << "Generated: " << out_file;
                cout << "\n"; 
            }
        }
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}