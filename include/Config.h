#pragma once
#include <limits>

inline constexpr int INF_LEVEL = 0x3f3f3f3f;

inline constexpr int NO_PARENT = -1;

inline constexpr int DEFAULT_SOURCE = 1;

enum class UpdateType : int
{
    INSERT = 0,
    DELETE = 1
};

enum class AlgorithmMode
{
    INCREMENTAL,
    DECREMENTAL,
    FULLY_DYNAMIC
};

enum class StorageMode
{
    FULL_SNAPSHOTS,
    SPACE_OPTIMIZED
};

enum class ErrorCorrectionMode
{
    NONE,
    TRIVIAL,  // if U_{i..j} == Û_{i..j} as sets, advance i to j
    NONTRIVIAL
};
