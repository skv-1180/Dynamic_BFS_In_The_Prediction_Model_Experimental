#!/usr/bin/env python3
import argparse
import os
import sys
import time
import shutil
import subprocess
import concurrent.futures
from pathlib import Path


def sanitize_filename(s):
    """
    Make a safe filename from testcase/mode/ec tag.
    """
    return (
        s.replace("/", "_")
         .replace("\\", "_")
         .replace(":", "_")
         .replace(" ", "_")
    )


def run_one(binary, tc_file, mode, runs, job_csv_time, job_csv_eta, timeout, use_ec):
    stem = Path(tc_file).stem
    ec_tag = "ec" if use_ec else "noec"
    tag = f"{stem}__{mode}__{ec_tag}"

    cmd = [
        binary,
        "--mode", mode,
        "--runs", str(runs),
        "--csv-time", job_csv_time,
        "--csv-eta", job_csv_eta,
        tc_file,
    ]

    if use_ec:
        cmd.append("--ec")

    t0 = time.time()
    try:
        # result = subprocess.run(
        #     cmd,
        #     capture_output=True,
        #     text=True,
        #     timeout=timeout
        # )
        result = subprocess.run(
            cmd,
            stdout=subprocess.PIPE,
            stderr=None,
            text=True,
            timeout=timeout
        )
        elapsed = time.time() - t0
        ok = result.returncode == 0

        stdout_text = result.stdout or ""

        stderr_line = ""
        stdout_line = stdout_text.strip().split("\n")[-1] if stdout_text.strip() else "" 

        return {
            "tag": tag,
            "ok": ok,
            "elapsed": elapsed,
            "stderr": stderr_line,
            "stdout": stdout_line,
            "returncode": result.returncode,
            "csv_time": job_csv_time,
            "csv_eta": job_csv_eta,
        }

    except subprocess.TimeoutExpired:
        return {
            "tag": tag,
            "ok": False,
            "elapsed": timeout,
            "stderr": "TIMEOUT",
            "stdout": "",
            "returncode": -1,
            "csv_time": job_csv_time,
            "csv_eta": job_csv_eta,
        }

    except Exception as e:
        return {
            "tag": tag,
            "ok": False,
            "elapsed": 0,
            "stderr": str(e),
            "stdout": "",
            "returncode": -2,
            "csv_time": job_csv_time,
            "csv_eta": job_csv_eta,
        }


def merge_csv_files(input_files, output_file):
    """
    Merge CSV files safely after all parallel jobs finish.

    Keeps the header from the first non-empty file.
    Skips repeated headers from later files.
    """
    input_files = [Path(p) for p in input_files if Path(p).exists() and Path(p).stat().st_size > 0]

    if not input_files:
        print(f"WARNING: no CSV files to merge for {output_file}")
        return

    output_file = Path(output_file)
    output_file.parent.mkdir(parents=True, exist_ok=True)

    header_written = False
    first_header = None

    with output_file.open("w", encoding="utf-8") as out:
        for csv_file in sorted(input_files):
            with csv_file.open("r", encoding="utf-8") as f:
                lines = f.readlines()

            if not lines:
                continue

            header = lines[0]

            if not header_written:
                out.write(header)
                first_header = header
                header_written = True

            for line in lines[1:]:
                # Avoid blank lines
                if not line.strip():
                    continue

                # Avoid repeated header lines
                if first_header is not None and line == first_header:
                    continue

                out.write(line)


def main():
    ap = argparse.ArgumentParser(description="Run simplified Dynamic BFS benchmarks")

    ap.add_argument("--binary", default="bin/benchmark")
    ap.add_argument("--test-dir", default="data/benchmark/testcases")
    ap.add_argument("--csv-time", default="results/time_vs_error.csv")
    ap.add_argument("--csv-eta", default="results/eta_scatter.csv")

    ap.add_argument(
        "--modes",
        nargs="+",
        default=["incremental", "decremental", "fullydynamic"]
    )

    ap.add_argument("--runs", type=int, default=5)
    ap.add_argument("--timeout", type=int, default=120)
    ap.add_argument("--parallel", type=int, default=1)
    ap.add_argument("--filter", default="")

    ap.add_argument(
        "--ec-modes",
        nargs="+",
        default=["off", "on"],
        choices=["off", "on"],
        help="Run without EC, with EC, or both"
    )

    ap.add_argument(
        "--keep-temp",
        action="store_true",
        help="Keep per-job temporary CSV files after merging"
    )

    args = ap.parse_args()

    test_dir = Path(args.test_dir)

    if not test_dir.exists():
        print(f"ERROR: test directory '{test_dir}' does not exist.")
        sys.exit(1)

    tc_files = sorted(test_dir.glob("*.txt"))

    if args.filter:
        tc_files = [f for f in tc_files if args.filter in str(f)]

    if not tc_files:
        print(f"No .txt files found in {test_dir} filter='{args.filter}'")
        sys.exit(1)

    # Final output directories
    csv_time_path = Path(args.csv_time)
    csv_eta_path = Path(args.csv_eta)

    csv_time_path.parent.mkdir(parents=True, exist_ok=True)
    csv_eta_path.parent.mkdir(parents=True, exist_ok=True)

    # Remove old final output files
    for p in [csv_time_path, csv_eta_path]:
        if p.exists():
            p.unlink()

    # Temporary per-job output directory
    temp_root = Path("results/parallel_tmp")
    temp_time_dir = temp_root / "time"
    temp_eta_dir = temp_root / "eta"

    if temp_root.exists():
        shutil.rmtree(temp_root)

    temp_time_dir.mkdir(parents=True, exist_ok=True)
    temp_eta_dir.mkdir(parents=True, exist_ok=True)

    jobs = []

    for tc in tc_files:
        modes_for_file = args.modes

        for m in ["incremental", "decremental", "fullydynamic"]:
            if f"_mode_{m}_" in tc.stem:
                modes_for_file = [m] if m in args.modes else []
                break

        for mode in modes_for_file:
            for ec_mode in args.ec_modes:
                use_ec = ec_mode == "on"

                stem = Path(tc).stem
                ec_tag = "ec" if use_ec else "noec"
                tag = f"{stem}__{mode}__{ec_tag}"
                safe_tag = sanitize_filename(tag)

                job_csv_time = str(temp_time_dir / f"{safe_tag}.csv")
                job_csv_eta = str(temp_eta_dir / f"{safe_tag}.csv")

                jobs.append({
                    "tc_file": str(tc),
                    "mode": mode,
                    "use_ec": use_ec,
                    "job_csv_time": job_csv_time,
                    "job_csv_eta": job_csv_eta,
                })

    print(f"Found {len(tc_files)} testcase file(s)")
    print(f"Total jobs: {len(jobs)}")
    print(f"Modes: {args.modes}")
    print(f"EC modes: {args.ec_modes}")
    print(f"Parallel workers: {args.parallel}")
    print(f"Temporary CSV dir: {temp_root}")

    ok_count = 0
    fail_count = 0

    completed_results = []

    def run_job(job):
        return run_one(
            args.binary,
            job["tc_file"],
            job["mode"],
            args.runs,
            job["job_csv_time"],
            job["job_csv_eta"],
            args.timeout,
            job["use_ec"],
        )

    if args.parallel > 1:
        with concurrent.futures.ThreadPoolExecutor(max_workers=args.parallel) as ex:
            futures = {ex.submit(run_job, j): j for j in jobs}

            for i, fut in enumerate(concurrent.futures.as_completed(futures), 1):
                r = fut.result()
                completed_results.append(r)

                status = "✓" if r["ok"] else "✗"
                print(f"[{i:3d}/{len(jobs)}] {status} {r['tag'][:90]}  {r['elapsed']:.1f}s")

                if r["stderr"]:
                    print(f"       {r['stderr']}")

                if r["ok"]:
                    ok_count += 1
                else:
                    fail_count += 1

    else:
        for i, job in enumerate(jobs, 1):
            r = run_job(job)
            completed_results.append(r)

            status = "✓" if r["ok"] else "✗"
            print(f"[{i:3d}/{len(jobs)}] {status} {r['tag'][:90]}  {r['elapsed']:.1f}s")

            if r["stderr"]:
                print(f"       {r['stderr']}")

            if r["ok"]:
                ok_count += 1
            else:
                fail_count += 1

    print("\nMerging CSV files...")

    time_csvs = [r["csv_time"] for r in completed_results if r["ok"]]
    eta_csvs = [r["csv_eta"] for r in completed_results if r["ok"]]

    merge_csv_files(time_csvs, csv_time_path)
    merge_csv_files(eta_csvs, csv_eta_path)

    if not args.keep_temp:
        shutil.rmtree(temp_root, ignore_errors=True)
    else:
        print(f"Kept temporary CSV files in: {temp_root}")

    print("\n" + "=" * 60)
    print(f"Done: {ok_count} succeeded, {fail_count} failed")
    print(f"time CSV: {csv_time_path}")
    print(f"eta  CSV: {csv_eta_path}")

    sys.exit(0 if fail_count == 0 else 1)


if __name__ == "__main__":
    main()