#pragma once
#include <vector>
#include "BFSState.h"
#include "Types.h"
#include <unordered_map>
using namespace std;

void buildPredEdgeIdx(unordered_map<EdgeUpdate, int, EdgeUpdateHash>& predEdgeIdx, const EdgeList& predictedUpdates);

// for decremental currently
void repairLevel(
    std::vector<std::unordered_set<int>>& LL, 
    int l, 
    vector<int>&level, 
    vector<int>&parent, 
    std::vector<std::unordered_set<int>>& UP,
    int n,
    const std::vector<std::unordered_set<int>>& prevInList,
    const std::vector<std::unordered_set<int>>& prevOutList
);


void repairLevel(BFSState& ws, std::vector<std::set<int>>& LL, int l);

void removeCommonEdges(EdgeList& E_del, EdgeList& E_ins);

BFSState buildWorkingState(const BFSSnapshot& snap,
                           const BFSState&    realState,
                           const EdgeList&    batchUpdates);


using ull = unsigned long long;
using Update = EdgeUpdate; // (u, v, type)

struct UpdateHashOnly {
    int sz = 0;
    ull h1 = 0;
    ull h2 = 0;

    static ull splitmix64(ull x) {
        x += 0x9e3779b97f4a7c15ULL;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9ULL;
        x = (x ^ (x >> 27)) * 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }

    static ull hashUpdate(const Update& up, ull seed) {
        auto [u, v, t] = up;

        ull h = seed;
        h ^= splitmix64((ull)u + 0x9e3779b97f4a7c15ULL);
        h ^= splitmix64((ull)v + 0xbf58476d1ce4e5b9ULL);
        h ^= splitmix64((ull)t + 0x94d049bb133111ebULL);

        return splitmix64(h);
    }

    static ull getHash1(const Update& up) {
        return hashUpdate(up, 123456789ULL);
    }

    static ull getHash2(const Update& up) {
        return hashUpdate(up, 987654321ULL);
    }

    void insert(const Update& up) {
        h1 ^= getHash1(up);
        h2 ^= getHash2(up);
        sz++;
    }

    void erase(const Update& up) {
        h1 ^= getHash1(up);
        h2 ^= getHash2(up);
        sz--;
    }

    bool sameSet(const UpdateHashOnly& other) const {
        return sz == other.sz && h1 == other.h1 && h2 == other.h2;
    }

    pair<ull, ull> hash() const {
        return {h1, h2};
    }

    int size() const {
        return sz;
    }
};
