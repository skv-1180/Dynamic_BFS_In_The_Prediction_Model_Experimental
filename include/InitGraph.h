#pragma once
// ============================================================
// InitGraph.h  –  reading ProblemInstance from file / stdin
// ============================================================
#include <string>
#include "Graph.h"

// Read from stdin (used when no filename is supplied).
Graph readGraph(std::istream& in);

// Read from a named file.  Throws std::runtime_error on failure.
Graph readGraphFromFile(const std::string& filename);
