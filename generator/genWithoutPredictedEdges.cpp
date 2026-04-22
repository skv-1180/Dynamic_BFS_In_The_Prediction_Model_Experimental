#include <bits/stdc++.h>
#include "erdosRenyi.h"

int main(){
    std::vector<std::array<int, 4>> test_cases = {
        {10, 20, 10, 0},
        {100, 200, 100, 0},
        {1000, 4000, 2000, 0},
        {1000, 10000, 5000, 0},
        {10000, 200000, 100000, 0},

        {10, 20, 10, 1},
        {100, 200, 100, 1},
        {1000, 4000, 2000, 1},
        {1000, 10000, 5000, 1},
        {10000, 200000, 100000, 1},

        {10, 20, 10, 2},
        {100, 200, 100, 2},
        {1000, 4000, 2000, 2},
        {1000, 10000, 5000, 2},
        {10000, 200000, 100000, 2}
    };

    std::string folder = "../testcases/erdosRenyi";
    for (const auto& [n, m0, m, mode] : test_cases) {
        DynamicGraphInstance instance;
        if(mode == 0) {
            instance = generate_incremental(n, m0, m);
        } else if(mode == 1) {
            instance = generate_decremental(n, m0, m);
        } else if(mode == 2) {
            instance = generate_fully_dynamic(n, m0, m);
        } else {
            cerr << "Invalid mode\n";
            continue;
        }
        int actual_m0 = (int)instance.initial_edges.size();
        int actual_m = (int)instance.updates.size();
        std::string filename = folder + "/n" + std::to_string(n) + "_m0_" + std::to_string(actual_m0) + "_m_" + std::to_string(actual_m) + "_mode_" + std::to_string(mode) + ".txt";
        std::ofstream out(filename);
        if (!out.is_open()) {
            cerr << "Failed to open file: " << filename << "\n";
            continue;
        }
        print_instance(instance, out);
        out.close();
    }
}

/* for testing
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