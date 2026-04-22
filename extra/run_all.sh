#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "$ROOT"

BINARY="$ROOT/bin/benchmark"
TEST_DIR="$ROOT/data/benchmark/testcases"
TIME_CSV="$ROOT/results/time_vs_error.csv"
ETA_CSV="$ROOT/results/eta_scatter.csv"
PLOT_DIR="$ROOT/results/plots"

echo "[1/3] Build benchmark"
make bench

echo "[2/3] Run benchmarks"
python3 tools/benchmark_runner.py \
  --binary "$BINARY" \
  --test-dir "$TEST_DIR" \
  --csv-time "$TIME_CSV" \
  --csv-eta "$ETA_CSV" \
  --modes incremental decremental fullydynamic \
  --runs 5 \
  --parallel 1

echo "[3/3] Plot"
python3 tools/plot_results.py \
  --time-csv "$TIME_CSV" \
  --eta-csv "$ETA_CSV" \
  --output-dir "$PLOT_DIR" \
  --format png

echo "Done."
echo "Time CSV : $TIME_CSV"
echo "Eta CSV  : $ETA_CSV"
echo "Plots    : $PLOT_DIR"