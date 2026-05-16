#include "genPredEdges.h"
using namespace std;
/*
22
0.0 0.02 0.05 0.1 0.15 0.2 0.25 0.3 0.35 0.4 0.45 0.5 0.55 0.6 0.65 0.7 0.75 0.8 0.85 0.9 0.95 0.99
21
0.0 0.02 0.05 0.1 0.15 0.2 0.25 0.3 0.35 0.4 0.45 0.5 0.55 0.6 0.65 0.7 0.75 0.8 0.85 0.9 0.99
13
0 0.01 0.02 0.05 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9
*/

int main() {
    try {
        // string folder = "testcases/erdos/withoutPredictedEdges";
        string folder = "fdTest/realGraphs";
        string out_folder = "fdTest/predReal";

        // string folder = "incdec/realGraphs/set4";
        // string out_folder = "incdec/predReal";
        std::cout << "folder: " << folder << std::endl;
        
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

        std::string subfolder;
        cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        cout << "Enter subfolder name: ";
        getline(cin, subfolder);

        filesystem::create_directories(out_folder);

        uint32_t base_seed = 
        std::random_device{}() ^
        std::chrono::high_resolution_clock::now().time_since_epoch().count();

        for (int fi = 0; fi < (int)files.size(); ++fi) {
            // std::string sout_folder =  out_folder+"/"+subfolder+to_string(fi);
            for (double p : error_rates) {
                int p1 = p*100;
                std::string sout_folder =  out_folder+"/"+subfolder+to_string(p1);
                filesystem::create_directories(sout_folder);
                for(int iter = 0; iter < 10; ++iter){
                    auto instance = read_instance(files[fi]);
    
                    // generatePredictedByStateError(instance, p, base_seed + fi * 1000 + (int)round(p * 1000));
                    generatePredictedByPositionError(instance, p, base_seed + fi * 1000 + (int)round(p * 1000));
                    
                    if((instance.initial_edges.size() + instance.realUpdates.size()) <= 10000){
                        double achievedErrorRate = computeAchievedStateErrorRate(instance);
                        p = achievedErrorRate;
                    }
    
                    ostringstream oss;
                    oss << fixed << setprecision(2) << p;
    
                    string out_file = sout_folder + "/graph_";
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
    
                    // out_file += "err" + oss.str() + "_" +
                    //                   filesystem::path(files[fi]).filename().string();
                    // out_file += "_iter" + to_string(iter)+".txt";
                    out_file += "err" + oss.str() + "_" +
                                filesystem::path(files[fi]).stem().string();

                    out_file += "_iter" + to_string(iter) + ".txt";
                    write_instance(out_file, instance, p);
    
                    cout << "Generated: " << out_file;
                    cout << "\n"; 
                }
            }
        }
    } catch (const exception& ex) {
        cerr << "Error: " << ex.what() << "\n";
        return 1;
    }

    return 0;
}