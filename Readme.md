# Dynamic BFS with Predictions

This project implements and benchmarks Breadth-First Search (BFS) graph processing workflows using a prediction model. It was developed as a B.Tech Project at IIT Roorkee by Shubham Kumar Verma and Utkarsh Lohiya, under the guidance of Prof. Shahbaz Khan.

## Project Structure
* **`include/`**: C++ headers and configurations.
* **`src/`**: Core C++ source code.
* **`tests/`**: Benchmarks and baseline metrics.
* **`tools/`**: Python testing and plotting scripts.
* **`docs/`**: Project report.

## Prerequisites
* C++20 compiler
* `make`
* `python3` (with dependencies listed in `requirements.txt`)

## Build Instructions
* **`make all`**: Builds release executable (`bin/app`).
* **`make debug`**: Builds debug executable with sanitizers (`bin/app_debug`).
* **`make bench`**: Builds benchmark suite (`bin/benchmark`).
* **`make clean`**: Removes compiled binaries and objects.

## Running the Application
Execute the main application on a test case by specifying the mode. Include `--ec` to enable non-trivial error correction:
```bash
bin/app --mode <incremental/decremental/fullydynamic> --quiet --ec <testcase_filename>
```

## Benchmarking & Plotting
Run a single test case for performance tracking:
```bash
./bin/benchmark --mode fullydynamic --ec --runs 1 --csv-time time.csv <testcase_filename>
```

Execute a full benchmark suite across a directory of test cases:
```bash
python3 tools/benchmark_runner.py \
  --binary bin/benchmark \
  --test-dir data/benchmark \
  --csv-time results/time_vs_error.csv \
  --modes incremental decremental fullydynamic \
  --runs 5 \
  --ec-modes off on
```

Generate evaluation plots from the output data:
```bash
python3 tools/plot_results.py \
  --output-dir results/plots \
  --time-csv results/time_vs_error.csv
```