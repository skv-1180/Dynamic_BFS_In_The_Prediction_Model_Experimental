# Dynamic BFS with Predictions

Reference implementation for the SWAT 2026 paper:
**"Dynamic Breadth-First Search with Predictions"**

---

## Algorithms Implemented

| Section | Class / Function | Time per update | Preprocessing |
|---------|------------------|-----------------|---------------|
| §3 | `IncrementalBFS` | O(eta_e + eta_v) | O(mn) |
| §4 | `DecrementalBFS` | O(min(m, eta_e + eta_v*)) | O(m^2) |
| §5 | `FullyDynamicBFS` | O(min(m, eta_e + eta_v*)) | O(m^2) |

---

## Directory Layout

```text
Dynamic_BFS_In_The_Prediction_Model_Experimental/
|-- makefile
|-- README.md
|-- include/
|   |-- Config.h
|   |-- Types.h
|   |-- Graph.h
|   |-- InitGraph.h
|   |-- BFSState.h
|   |-- BFSAlgorithms.h
|   |-- Preprocess.h
|   |-- Incremental.h
|   |-- Decremental.h
|   `-- FullyDynamic.h
|-- src/
|   |-- Graph.cpp
|   |-- InitGraph.cpp
|   |-- BFSState.cpp
|   |-- BFSAlgorithms.cpp
|   |-- Preprocess.cpp
|   |-- Incremental.cpp
|   |-- Decremental.cpp
|   |-- FullyDynamic.cpp
|   `-- main.cpp
`-- data/
    |-- example_incremental.txt
    |-- example_decremental.txt
    `-- example_fullydynamic.txt
```

---

## Building

Requires **g++ with C++20** support.

```bash
make
make debug
make clean
```

---

## Running

```bash
./bin/app [options] <inputfile>
```

| Option | Values | Default | Description |
|--------|--------|---------|-------------|
| `--mode` | `incremental` \| `decremental` \| `fullydynamic` | `fullydynamic` | Algorithm variant |
| `--store` | `full` | `full` | Snapshot storage |
| `--verify` | flag | off | Cross-check every result against a fresh BFS |
| `--quiet` | flag | off | Suppress per-step BFS tree output |

### Quick examples

```bash
./bin/app --mode incremental --verify data/example_incremental.txt
./bin/app --mode decremental --quiet data/example_decremental.txt
./bin/app --mode fullydynamic data/example_fullydynamic.txt
```

### Makefile convenience targets

```bash
make run_inc
make run_dec
make run_fd
make verify
```

---

## Input File Format

```text
# Lines beginning with '#' are comments
numVertices  source  numInitialEdges  numUpdates
u1 v1               <- initial edges
...
u v type            <- predicted updates (0=insert, 1=delete)
...
u v type            <- real updates
...
```

All vertices are 1-indexed and the graph is directed.

---

## Architecture Notes

| Layer | Role |
|-------|------|
| `BFSState` | Mutable working copy: adjacency, `level[]`, `parent[]`, `UP[]` |
| `BFSAlgorithms` | Pure batch algorithms operating on a `BFSState` |
| `Preprocess` | Offline simulation of the predicted sequence into `vector<BFSSnapshot>` |
| `Incremental/Decremental/FullyDynamic` | Online wrappers that track the real graph and dispatch Case 1 / Case 2 |

### Case 1 vs Case 2

- Case 1: the real update matches the next prediction, so the algorithm serves the precomputed snapshot directly.
- Case 2: the sequence diverges, so the algorithm rebuilds a working state from the last matched snapshot and repairs the BFS tree using the unmatched real updates.

### Testing

- Add a new `.txt` case under `data/` and run with `--verify`.
- To benchmark against classical ES-trees, compare `Case-2 (batch)` time with a standalone BFS run.
