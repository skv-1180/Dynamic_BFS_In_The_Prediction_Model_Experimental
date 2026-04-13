#pragma once
#include <string>

#include "Graph.h"

Graph readGraph(std::istream& in);
Graph readGraphFromFile(const std::string& filename);
