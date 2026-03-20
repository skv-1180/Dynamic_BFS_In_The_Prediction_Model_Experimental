#pragma once

// ============================================================
// Config.h
// Global compile-time constants, enums and tuneable parameters.
// ============================================================

#include <limits>

// A level value used to represent "vertex is unreachable from source".
inline constexpr int INF_LEVEL = 0x3f3f3f3f;

// Sentinel for "no parent" in the BFS tree.
inline constexpr int NO_PARENT = -1;

// Default source vertex (1-indexed throughout the project).
inline constexpr int DEFAULT_SOURCE = 1;

// Edge-update types
enum class UpdateType : int {
    INSERT = 0,   // edge insertion
    DELETE = 1    // edge deletion
};

// Which algorithm variant to run
enum class AlgorithmMode {
    INCREMENTAL,    // all real updates are insertions
    DECREMENTAL,    // all real updates are deletions
    FULLY_DYNAMIC   // real updates may be insertions or deletions
};

// Which storage strategy to use for the precomputed predicted trees
enum class StorageMode {
    FULL_SNAPSHOTS
};
