# Experimental Graph Update Benchmark

This project implements and benchmarks graph processing workflows for:

- **Incremental**
- **Decremental**
- **Fully Dynamic**

## Project Structure

- `src/` — core source code
- `tests/` — benchmark and helper code
- `tools/` — benchmark runner and plotting scripts
- `data/` — testcase inputs
- `bin/` — compiled executables
- `results/` — benchmark outputs and plots

## Prerequisites

- `make`
- `g++` or a compatible C++ compiler
- `python3`
- Python packages listed in `requirements.txt`

## Build

```bash
make clean
make
make bench
```

## Run the Main Application

```bash
bin/app --mode <incremental/decremental/fullydynamic> --quiet --ec <testcase_filename>
```

### Options

- `--quiet` is optional
- `--ec` is optional
- `--ec` enables nontrivial error correction

### Examples

```bash
bin/app --mode incremental --quiet example_incremental.txt
bin/app --mode decremental --quiet example_decremental.txt
bin/app --mode fullydynamic --quiet example_fullydynamic.txt
bin/app --mode fullydynamic --quiet --ec example_fullydynamic.txt
```

## Run a Single Benchmark Testcase

```bash
./bin/benchmark --mode <incremental/decremental/fullydynamic> --runs <no_of_runs> --csv-time <filename.csv> <testcase_filename>
```

### Example

```bash
./bin/benchmark --mode fullydynamic --ec --runs 1 --csv-time time.csv data/benchmark/testcasesReal2/graph_mode_fullydynamic_err0.25_graph_subset_n500_m2000_mode_2_.txt
```

## Run Benchmarks on a Folder

```bash
python3 tools/benchmark_runner.py \
  --binary bin/benchmark \
  --test-dir data/benchmark/testcases \
  --csv-time results/time_vs_error.csv \
  --modes incremental decremental fullydynamic \
  --runs 5 \
  --ec-modes off on
```

## Plot Results

Install Python dependencies first:

```bash
python -m pip install --upgrade pip
python -m pip install -r requirements.txt
```

Then generate plots:

```bash
python3 tools/plot_results.py \
  --output-dir results/plots \
  --time-csv results/time_vs_error.csv
```

## Notes

- Ensure the input testcase file exists before running app or benchmark.
- Benchmark results are written to the CSV file specified by `--csv-time`.
- Plots are saved to the directory given by `--output-dir`.
