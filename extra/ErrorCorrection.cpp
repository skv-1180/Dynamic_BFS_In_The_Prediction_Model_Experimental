// ============================================================
// ErrorCorrection.cpp
// ============================================================

#include "../include/ErrorCorrection.h"
#include <map>
#include <algorithm>

using FreqKey = std::pair<std::pair<int,int>,int>;
using FreqMap = std::map<FreqKey, int>;

static FreqMap makeFreqMap(const EdgeList& batch)
{
    FreqMap freq;
    for (const auto& e : batch)
        ++freq[{{e.u, e.v}, (int)e.type}];
    return freq;
}

// ===========================================================
bool trivialErrorCorrection(
    int /*i*/, int j,
    const EdgeList& realBatch,
    const EdgeList& predictedBatch,
    int& newLastMatched)
{
    if (realBatch.size() != predictedBatch.size()) return false;
    if (makeFreqMap(realBatch) == makeFreqMap(predictedBatch)) {
        newLastMatched = j;
        return true;
    }
    return false;
}

// ===========================================================
EdgeList nontrivialErrorCorrectionIncremental(
    int i,
    const EdgeList& realBatch,
    const EdgeList& predictedBatch,
    int& bestK)
{
    FreqMap avail = makeFreqMap(realBatch);
    int matched = 0;
    for (const auto& e : predictedBatch) {
        FreqKey key = {{e.u, e.v}, (int)e.type};
        auto it = avail.find(key);
        if (it == avail.end() || it->second == 0) break;
        --it->second;
        ++matched;
    }
    bestK = i + matched;

    FreqMap consumed;
    for (int idx = 0; idx < matched; ++idx) {
        const auto& e = predictedBatch[idx];
        ++consumed[{{e.u, e.v}, (int)e.type}];
    }

    EdgeList remaining;
    for (const auto& e : realBatch) {
        FreqKey key = {{e.u, e.v}, (int)e.type};
        auto it = consumed.find(key);
        if (it != consumed.end() && it->second > 0)
            --it->second;
        else
            remaining.push_back(e);
    }
    return remaining;
}

// ===========================================================
EdgeList nontrivialErrorCorrectionDecremental(
    int i,
    const EdgeList& realBatch,
    const EdgeList& predictedBatch,
    int& bestK)
{
    return nontrivialErrorCorrectionIncremental(i, realBatch, predictedBatch, bestK);
}

// ===========================================================
ErrorCorrectionResult nontrivialErrorCorrectionFullyDynamic(
    int i,
    const EdgeList& realBatch,
    const EdgeList& predictedBatch)
{
    int predN = (int)predictedBatch.size();
    int realN = (int)realBatch.size();

    FreqMap realFreq = makeFreqMap(realBatch);

    int bestSize = realN; // k=i: no predicted prefix, cost = realN
    int bestK    = i;
    int matchedCount = 0;

    for (int k = 0; k < predN; ++k) {
        const auto& e = predictedBatch[k];
        FreqKey key = {{e.u, e.v}, (int)e.type};
        if (realFreq.count(key) && realFreq[key] > 0) {
            --realFreq[key];
            ++matchedCount;
        }
        int cost = (k + 1 - matchedCount) + (realN - matchedCount);
        if (cost < bestSize) {
            bestSize = cost;
            bestK    = i + k + 1;
        }
    }

    // Rebuild corrected batch for bestK
    int bestPrefixLen = bestK - i;
    FreqMap availReal = makeFreqMap(realBatch);
    FreqMap matchFreq;

    for (int k = 0; k < bestPrefixLen; ++k) {
        const auto& e = predictedBatch[k];
        FreqKey key = {{e.u, e.v}, (int)e.type};
        if (availReal.count(key) && availReal[key] > 0) {
            --availReal[key];
            ++matchFreq[key];
        }
    }

    EdgeList corrDel, corrIns;
    FreqMap consumeP = matchFreq;

    for (int k = 0; k < bestPrefixLen; ++k) {
        const auto& e = predictedBatch[k];
        FreqKey key = {{e.u, e.v}, (int)e.type};
        if (consumeP.count(key) && consumeP[key] > 0) {
            --consumeP[key];
        } else {
            if (e.type == UpdateType::INSERT)
                corrDel.push_back({e.u, e.v, UpdateType::DELETE});
            else
                corrIns.push_back({e.u, e.v, UpdateType::INSERT});
        }
    }

    FreqMap consumeA = matchFreq;
    for (const auto& e : realBatch) {
        FreqKey key = {{e.u, e.v}, (int)e.type};
        if (consumeA.count(key) && consumeA[key] > 0) {
            --consumeA[key];
        } else {
            if (e.type == UpdateType::INSERT)
                corrIns.push_back(e);
            else
                corrDel.push_back(e);
        }
    }

    ErrorCorrectionResult res;
    res.bestK            = bestK;
    res.correctedDeletes = corrDel;
    res.correctedInserts = corrIns;
    res.originalEtaE     = realN;
    res.correctedEtaE    = (int)corrDel.size() + (int)corrIns.size();
    return res;
}
