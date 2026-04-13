#pragma once

inline constexpr int INF_LEVEL = 0x3f3f3f3f;
inline constexpr int NO_PARENT = -1;
inline constexpr int DEFAULT_SOURCE = 1;
// (1-indexed throughout the project)

enum class UpdateType : int {
    INSERT = 0,
    DELETE = 1
};

// Which algorithm mode to run
enum class AlgorithmMode {
    INCREMENTAL,
    DECREMENTAL,
    FULLY_DYNAMIC
};

enum class StorageMode {
    FULL_SNAPSHOTS
};
