#pragma once

// ============================================================
// ErrorCorrection.h
// Error correction mechanisms (Section 7 of the paper).
//
// Three variants:
//   TRIVIAL     - if U_{i..j} == Û_{i..j} as a SET, advance i to j.
//   NONTRIVIAL  - find best k that minimises corrected batch size.
// ============================================================

#include <vector>
#include "Types.h"
#include "Config.h"

// Result of running non-trivial fully-dynamic error correction.
struct ErrorCorrectionResult {
    int      bestK{};
    EdgeList correctedDeletes;
    EdgeList correctedInserts;
    int      originalEtaE{};
    int      correctedEtaE{};
};

// TRIVIAL: checks whether U_{i..j} == Û_{i..j} as multisets.
// Returns true and sets newLastMatched = j when it fires.
bool trivialErrorCorrection(
    int             i,
    int             j,
    const EdgeList& realBatch,
    const EdgeList& predictedBatch,
    int&            newLastMatched
);

// NON-TRIVIAL INCREMENTAL: find best matched prefix of Û in U,
// returns the remaining (unmatched) real batch U_{k..j}.
EdgeList nontrivialErrorCorrectionIncremental(
    int             i,
    const EdgeList& realBatch,
    const EdgeList& predictedBatch,
    int&            bestK
);

// NON-TRIVIAL DECREMENTAL: same logic.
EdgeList nontrivialErrorCorrectionDecremental(
    int             i,
    const EdgeList& realBatch,
    const EdgeList& predictedBatch,
    int&            bestK
);

// NON-TRIVIAL FULLY-DYNAMIC: find k in [i,j] that minimises
// |E*_del| + |E*_ins|.  Total cost O(η_e).
ErrorCorrectionResult nontrivialErrorCorrectionFullyDynamic(
    int             i,
    const EdgeList& realBatch,
    const EdgeList& predictedBatch
);
