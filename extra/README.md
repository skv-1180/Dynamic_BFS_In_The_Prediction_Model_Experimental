# Dynamic BFS with Predictions — Experimental Implementation

Reference implementation for:
> **"Dynamic Breadth-First Search with Predictions"**  
> Submitted to SWAT 2026 (anonymous review)

---

## Table of Contents
1. [Overview](#1-overview)
2. [Algorithms Implemented](#2-algorithms-implemented)
3. [Project Structure](#3-project-structure)
4. [Building](#4-building)
5. [Running — Normal Mode](#5-running--normal-mode)
6. [Running — Benchmark Mode](#6-running--benchmark-mode)
7. [Generating Test Cases](#7-generating-test-cases)
8. [Running Benchmarks](#8-running-benchmarks)
9. [Plotting Results](#9-plotting-results)
10. [Full Pipeline (run_all.sh)](#10-full-pipeline-run_allsh)
11. [Downloading Real Graphs](#11-downloading-real-graphs)
12. [Input File Format](#12-input-file-format)
13. [What We Measure and How](#13-what-we-measure-and-how)
14. [Prediction Strategies](#14-prediction-strategies)
15. [Classical Algorithms (IITD Lecture Notes)](#15-classical-algorithms-iitd-lecture-notes)
16. [Space-Optimized Decremental (§new)](#16-space-optimized-decremental-new)
17. [Flags Quick Reference](#17-flags-quick-reference)

---

## 1. Overview

Five prediction-augmented BFS algorithms (plus classical baselines) are implemented.
Given a directed graph G=(V,E) with source s, a **predicted** update sequence Û is
preprocessed offline. At runtime, real updates arrive one at a time:

- **Case 1** (sequences agree): serve directly from precomputed snapshot — O(1).
- **Case 2** (sequences diverged): run the appropriate batch algorithm on a temporary
  working state built from the last valid snapshot T̂_i.

---

## 2. Algorithms Implemented

| # | Class | Mode | Paper Section | Preprocessing | Per-update |
|---|-------|------|--------------|---------------|------------|
| 1 | `IncrementalBFS` | insertions only | §3 | O(mn) | O(η_e + η_v) |
| 2 | `SpaceOptimizedIncremental` | insertions only | §6 | O(mn) time, O(n²) space | O(n log n + η_e + η_v) |
| 3 | `DecrementalBFS` | deletions only | §4 | O(m²) | O(min{m, η_e + η*_v}) |
| 4 | `SpaceOptimizedDecremental` | deletions only | §new | O(mn/k) space | O(min{m, η_e + η*_v}) |
| 5 | `FullyDynamicBFS` | insert + delete | §5 | O(m²) | O(min{m, η_e + η*_v}) |

**Error measures** (relative to last matched snapshot at step i, evaluated at step j):

| Symbol | Definition |
|--------|------------|
| η_e | j − i (mis-predicted updates since last match) |
| η_v | Σ deg(v) over vertices where L̂_i[v] ≠ L_j[v] |
| η*_v | Σ deg(v)·\|L_j[v] − L̂_i[v]\| |

**Classical baselines** (IITD Lecture Notes algorithms, single-edge-update):

| Function | Description |
|----------|-------------|
| `timeClassicalIncremental` | IITD incremental: level-by-level propagation |
| `timeClassicalDecremental` | IITD decremental: A_v cursor-based repair |
| `timeClassicalFullyDynamic` | IITD insert/delete interleaved |
| `timeNaiveFullyDynamic` | Full BFS recomputation per update, O(n+m) |

---

## 3. Project Structure

```
Dynamic_BFS_In_The_Prediction_Model_Experimental/
├── makefile
├── README.md
├── include/
│   ├── Config.h, Types.h, Graph.h, InitGraph.h
│   ├── BFSState.h          ← adjacency + level[] + parent[] + UP[]
│   ├── BFSAlgorithms.h     ← classicalInsert/Delete, batch algorithms
│   ├── Preprocess.h        ← offline snapshot preprocessing
│   ├── Incremental.h       ← §3 IncrementalBFS
│   ├── Decremental.h       ← §4 DecrementalBFS
│   ├── FullyDynamic.h      ← §5 FullyDynamicBFS
│   ├── SpaceOptimized.h    ← §6 SpaceOptimizedIncremental
│   ├── SpaceOptimizedDecremental.h  ← §new
│   ├── ErrorCorrection.h   ← §7 trivial + nontrivial EC
│   ├── Metrics.h           ← η_e, η_v, η*_v (outside timing)
│   └── ClassicalBFS.h      ← IITD timing wrappers
├── src/                    ← mirror of include/ with .cpp
├── tools/
│   ├── generate_testcases.py
│   ├── benchmark_runner.py
│   ├── plot_results.py
│   ├── run_all.sh
│   └── requirements.txt
├── data/
│   ├── example_*.txt       ← small test cases
│   └── benchmark/
│       ├── graphs/         ← cached SNAP downloads
│       └── testcases/      ← generated benchmark files
└── results/
    ├── benchmark.csv, raw/*.json, plots/*.png
```

---

## 4. Building

Requires g++ with C++20 (GCC 10+ or Clang 12+) and GNU Make.

```bash
make          # release build → bin/app
make debug    # ASan/UBSan debug build → bin/app_debug
make clean
make verify   # build + run 4 correctness checks
```

---

## 5. Running — Normal Mode

```bash
bin/app [OPTIONS] <inputfile>
```

| Option | Values | Default | Description |
|--------|--------|---------|-------------|
| `--mode` | `incremental` \| `decremental` \| `fullydynamic` | `fullydynamic` | Algorithm variant |
| `--ec` | `none` \| `trivial` \| `nontrivial` | `nontrivial` | Error correction §7 |
| `--store` | `full` \| `space` | `full` | `space` = space-optimized version |
| `--checkpoint-k` | int | 1 | Checkpoint interval for `--store space --mode decremental` |
| `--verify` | flag | off | Cross-check vs fresh BFS |
| `--quiet` | flag | off | Suppress per-step output |

```bash
# Incremental with correctness check
bin/app --mode incremental --verify data/example_incremental.txt

# Space-optimized decremental, checkpoint every 5 steps (O(mn/5) space)
bin/app --mode decremental --store space --checkpoint-k 5 data/example_decremental.txt

# Fully dynamic with non-trivial error correction
bin/app --mode fullydynamic --ec nontrivial data/example_fullydynamic.txt
```

---

## 6. Running — Benchmark Mode

```bash
bin/app --benchmark [OPTIONS] <inputfile>
```

Adds:

| Option | Values | Default | Description |
|--------|--------|---------|-------------|
| `--benchmark` | flag | — | Enable benchmark mode |
| `--all-algos` | flag | — | Run all 5 algorithms in one pass |
| `--n-runs` | int | 5 | Timing reps (minimum reported) |
| `--json-out` | file | stdout | Write JSON result |
| `--csv-append` | file | — | Append CSV row |
| `--no-naive` | flag | off | Skip naive O(m) baseline |
| `--no-preprocess-time` | flag | off | Skip preprocessing timing |
| `--checkpoint-k` | int | 1 | Space-opt decremental checkpoint interval |

**`--benchmark` writes JSON to stdout, summary to stderr. Redirect separately:**

```bash
# All 5 algorithms on one file
bin/app --benchmark --all-algos --n-runs 5 --no-naive \
    --csv-append results/benchmark.csv \
    --json-out results/raw/test1.json \
    data/benchmark/testcases/graph_ca-grqc_mode_fullydynamic_err0.20_case00_m200.txt

# Single algorithm
bin/app --benchmark --mode incremental --n-runs 10 \
    --csv-append results/benchmark.csv \
    data/example_incremental.txt

# Space-optimized decremental, k=10 checkpoint interval
bin/app --benchmark --mode decremental --store space --checkpoint-k 10 \
    --n-runs 5 data/example_decremental.txt
```

---

## 7. Generating Test Cases

```bash
python3 tools/generate_testcases.py [OPTIONS]
```

| Option | Default | Description |
|--------|---------|-------------|
| `--graph NAME` | `ca-grqc` | SNAP dataset or `synthetic` |
| `--synthetic-n N` | 500 | Vertices |
| `--synthetic-m M` | 4 | BA edges/node |
| `--n-updates N` | 200 | Updates per case |
| `--error-rates r ...` | `0.0 0.05 0.1 0.15 0.2 0.3 0.5 0.7 1.0` | Error rates |
| `--n-cases N` | 3 | Seeds per error rate |
| `--strategy STR` | `prefix` | See §14 |
| `--mode STR` | `fullydynamic` | Algorithm mode |
| `--output-dir DIR` | `data/benchmark/testcases` | Output |
| `--cache-dir DIR` | `data/benchmark/graphs` | Graph cache |
| `--seed N` | 42 | Random seed |

```bash
# Standard benchmark (prefix strategy recommended)
python3 tools/generate_testcases.py --graph ca-grqc --n-updates 500 \
    --n-cases 5 --strategy prefix --mode fullydynamic

# Error-correction demo (shuffle_ec shows §7 benefit)
python3 tools/generate_testcases.py --graph ca-grqc --n-updates 500 \
    --n-cases 5 --strategy shuffle_ec --mode fullydynamic

# All three modes
for mode in incremental decremental fullydynamic; do
  python3 tools/generate_testcases.py --graph ca-grqc \
      --n-updates 300 --n-cases 5 --strategy prefix --mode $mode
done
```

---

## 8. Running Benchmarks

```bash
python3 tools/benchmark_runner.py [OPTIONS]
```

| Option | Default | Description |
|--------|---------|-------------|
| `--binary` | `bin/app` | Binary path |
| `--test-dir` | `data/benchmark/testcases` | Test cases directory |
| `--output-csv` | `results/benchmark.csv` | Output CSV |
| `--modes m...` | all three | Algorithm modes |
| `--ec-modes e...` | `nontrivial` | Error correction |
| `--n-runs N` | 5 | Timing reps |
| `--json-dir` | `results/raw` | Per-case JSON |
| `--parallel N` | 1 | Workers |
| `--filter STR` | — | Filename filter |
| `--all-algos` | true | Run all 5 algorithms |
| `--checkpoint-k K` | 1 | Space-opt decremental checkpoint |
| `--no-naive` | flag | Skip naive baseline |
| `--timeout N` | 120s | Per-run timeout |

```bash
# Full benchmark, all 5 algorithms, parallel
python3 tools/benchmark_runner.py --n-runs 10 --no-naive --parallel 4

# Only fully dynamic with both EC modes compared
python3 tools/benchmark_runner.py --modes fullydynamic \
    --ec-modes none nontrivial --filter "strategy_prefix"

# Space-optimized decremental with k=5
python3 tools/benchmark_runner.py --modes decremental \
    --checkpoint-k 5 --filter "graph_ca-grqc"
```

---

## 9. Plotting Results

```bash
python3 tools/plot_results.py [OPTIONS]
```

| Option | Default | Description |
|--------|---------|-------------|
| `--csv` | `results/benchmark.csv` | Input CSV |
| `--output-dir` | `results/plots` | Plot directory |
| `--format` | `png` | `png` \| `pdf` \| `svg` |
| `--show` | flag | Open interactive windows |
| `--skip` | — | Plot names to skip |

Plots generated (numbered labels 1–5 for algorithms):

| File | Content |
|------|---------|
| `speedup_vs_error_rate` | Speedup vs error rate with 95% CI |
| `runtime_vs_error_rate` | Absolute online µs vs error rate |
| `case1_fraction` | Prediction hit rate (Case-1 %) |
| `eta_vs_error_rate` | η_e and η*_v vs error rate |
| `speedup_boxplot` | Box plots by error tier |
| `speedup_heatmap` | Heatmap (error_rate × algorithm) |
| `cdf_speedup` | CDF of speedup across all cases |
| `preprocess_space_comparison` | Preprocessing cost bar chart |
| `scatter_speedup_accuracy` | Speedup vs prediction accuracy |

---

## 10. Full Pipeline (run_all.sh)

```bash
bash tools/run_all.sh [OPTIONS]
```

| Option | Default | Description |
|--------|---------|-------------|
| `--graph` | `ca-grqc` | Graph |
| `--strategy` | `prefix` | Prediction strategy |
| `--checkpoint-k K` | 1 | Dec. space-opt checkpoint interval |
| `--n-updates N` | 200 | Updates per case |
| `--error-rates r...` | `0.0 0.05 0.1 0.15 0.2 0.3 0.5 0.7 1.0` | Error rates |
| `--n-cases N` | 4 | Seeds per rate |
| `--n-runs N` | 5 | Timing reps |
| `--no-naive` | flag | Skip naive baseline |
| `--parallel N` | 1 | Workers |
| `--plot-format EXT` | `png` | Plot format |
| `--skip-generate` | flag | Reuse existing cases |
| `--skip-benchmark` | flag | Just re-plot |

**Recommended for publication:**
```bash
# Using real graph (downloads ca-grqc ~5k nodes, ~14k edges)
bash tools/run_all.sh \
    --graph ca-grqc \
    --n-updates 500 \
    --n-cases 5 \
    --strategy prefix \
    --n-runs 10 \
    --no-naive \
    --parallel 4 \
    --plot-format pdf

# Demonstrate error correction (§7): use shuffle_ec strategy
bash tools/run_all.sh --graph ca-grqc --strategy shuffle_ec \
    --n-updates 500 --n-cases 5 --n-runs 10 --no-naive

# Space-time tradeoff for decremental: compare k=1,5,10
for K in 1 5 10; do
  bash tools/run_all.sh --strategy prefix --checkpoint-k $K \
      --skip-generate --plot-format pdf
done
```

---

## 11. Downloading Real Graphs

Graphs come from **Stanford SNAP** (https://snap.stanford.edu/data/).

**Automatic** (downloads and caches on first use):
```bash
python3 tools/generate_testcases.py --graph ca-grqc
```

**Manual** (if firewall blocks download):
```bash
mkdir -p data/benchmark/graphs && cd data/benchmark/graphs
wget https://snap.stanford.edu/data/ca-GrQc.txt.gz
mv ca-GrQc.txt.gz ca-grqc.txt.gz     # lowercase name required
```

Or via browser: https://snap.stanford.edu/data/ca-GrQc.html → click Download.

| Name | Nodes | Edges | Description |
|------|-------|-------|-------------|
| `ca-grqc` | 5,242 | 14,496 | General Relativity co-authorship |
| `ca-hepth` | 9,877 | 25,998 | HEP Theory co-authorship |
| `ca-hepph` | 12,008 | 118,521 | HEP Phenomenology |
| `ca-condmat` | 23,133 | 93,497 | Condensed Matter |
| `email-enron` | 36,692 | 183,831 | Enron email network |
| `p2p-gnutella04` | 10,876 | 39,994 | Peer-to-peer network |

---

## 12. Input File Format

```
# Comment lines start with #
numVertices  source  numInitialEdges  numUpdates
u v           ← initial edges (directed u→v)
...
u v type      ← predicted updates (0=insert, 1=delete)
...
u v type      ← real updates (same format)
...
```

All vertices 1-indexed. Graph is directed.

---

## 13. What We Measure and How

**Timing:** `std::chrono::high_resolution_clock`, N runs, minimum total reported.

**Key separation:** Preprocessing and online phases are timed independently.
The benchmark constructor accepts pre-built snapshots so preprocessing cost is
excluded from online timing. Metric computation (η_e, η_v, η*_v) is done in
a dedicated untimed pass.

**CSV key columns:**

| Column | Description |
|--------|-------------|
| `algo` | Algorithm (1–5) |
| `prediction_accuracy` | Case-1 count / total updates |
| `avg_eta_e`, `avg_eta_v`, `avg_eta_v_star` | Mean error measures per Case-2 |
| `classical_total_us` | IITD classical algorithm total time |
| `preprocess_us` | Preprocessing time |
| `online_total_us` | Prediction online phase time |
| `speedup_online_vs_classical` | classical / online |
| `speedup_total_vs_classical` | classical / (preprocess + online) |

---

## 14. Prediction Strategies

| Strategy | Description | Use case |
|----------|-------------|----------|
| `perfect` | Predicted == real, η_e = 0 | Verify O(1) best case |
| `prefix` | First (1−err)×m correct, rest random. Accuracy = (1−err)×100% | **Main benchmarking** |
| `shuffle_ec` | Prefix correct + 60% of errors are real updates in wrong order + 40% random. Demonstrates §7 error correction. | **EC demonstration** |
| `iid` | Each update corrupted independently | Noise modeling |
| `batch` | Errors in contiguous block | Prediction model failure |
| `shuffle` | Permute error fraction | Order uncertainty |

**Why `shuffle_ec` benefits error correction:**
With `shuffle_ec` at 30% error rate (m=100):
- First 70 predicted: correct prefix
- Next 18 predicted: real updates from positions 70–87, shuffled (correct edges, wrong order)
- Next 12 predicted: truly random
- **Non-trivial EC** can match the 18 misordered-but-correct updates back to the real
  sequence, advancing the last-matched pointer from i=70 toward 88 → fewer Case-2 batches.

---

## 15. Classical Algorithms (IITD Lecture Notes)

The classical timing functions implement exactly the algorithms from IITD lecture notes
on incremental and decremental BFS (single-edge-update, not batch):

**Incremental (IITD §2 Algorithm 1):**
Insert (x→y): if dist(y) > dist(x)+1, set parent(y)=x, dist(y)=dist(x)+1,
then propagate level-by-level using Li sets.

**Decremental (IITD §2 Algorithm 1):**
Delete (u→v): if u=parent(v), find T(v) (subtree at v), set dist=∞ for all vertices
in T(v), then repair level-by-level: for each y at level i, scan A_y (active
in-neighbors) for x with dist(x) < i; if found → new parent; if A_y empty →
move y to level i+1, reset A_y = N_y.

**Fully Dynamic:** Apply IITD insert or IITD delete as appropriate per update,
maintaining A_v arrays across both insert and delete operations.

These are provably O(mn) total time with O(n) amortized per-update cost.

---

## 16. Space-Optimized Decremental (§new)

Replaces the O(m²) UP-set storage of §4 with per-vertex pointer checkpoints:

**Checkpoint:** Stored every k steps. Each checkpoint contains level[], parent[],
and ptr[v] (cursor index into In[v]) for all vertices. Space: O(n) per checkpoint,
O(mn/k) total.

**Cursor invariant (§new):** ptr[v] points to the first unexamined in-neighbor of v
at the current BFS level. Advancing the cursor finds a replacement parent.
When the cursor is exhausted, v moves up one level and the cursor resets.

**Checkpoint interval k (--checkpoint-k K):**
- k=1 (default): every step stored, O(mn) space, instant recovery.
- k>1: O(mn/k) space; recovery replays at most k−1 deletions forward from checkpoint.

---

## 17. Flags Quick Reference

```bash
# ── Normal mode ───────────────────────────────────────────────
bin/app --mode  incremental|decremental|fullydynamic
        --ec    none|trivial|nontrivial
        --store full|space
        --checkpoint-k K      (k for dec. space-opt, default 1)
        --verify --quiet
        <inputfile>

# ── Benchmark mode ────────────────────────────────────────────
bin/app --benchmark
        --all-algos            (run all 5 in one pass)
        --mode  incremental|decremental|fullydynamic
        --store full|space
        --checkpoint-k K
        --n-runs N
        --json-out FILE
        --csv-append FILE
        --no-naive
        --no-preprocess-time
        <inputfile>

# ── Generate test cases ───────────────────────────────────────
python3 tools/generate_testcases.py
        --graph  ca-grqc|ca-hepth|...|synthetic
        --synthetic-n N  --synthetic-m M
        --n-updates N
        --error-rates r1 r2 ...
        --n-cases N
        --strategy  perfect|prefix|shuffle_ec|iid|batch|shuffle
        --mode  incremental|decremental|fullydynamic
        --output-dir DIR  --cache-dir DIR  --seed N

# ── Run benchmarks ────────────────────────────────────────────
python3 tools/benchmark_runner.py
        --binary bin/app  --test-dir DIR  --output-csv FILE
        --modes m1 m2 ...  --ec-modes e1 e2 ...
        --n-runs N  --parallel N  --filter STR
        --all-algos  --checkpoint-k K
        --no-naive  --no-preprocess-time  --timeout N

# ── Plot results ──────────────────────────────────────────────
python3 tools/plot_results.py
        --csv FILE  --output-dir DIR  --format png|pdf|svg
        --show  --skip speedup|runtime|case1|eta|boxplot|heatmap|cdf|space|scatter

# ── Full pipeline ─────────────────────────────────────────────
bash tools/run_all.sh
        --graph NAME  --strategy STR  --checkpoint-k K
        --n-updates N  --error-rates r...  --n-cases N
        --n-runs N  --no-naive  --parallel N
        --plot-format EXT  --skip-generate  --skip-benchmark

# ── Python dependencies ───────────────────────────────────────
pip install -r tools/requirements.txt
```
