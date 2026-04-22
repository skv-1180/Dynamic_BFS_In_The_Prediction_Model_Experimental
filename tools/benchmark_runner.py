#!/usr/bin/env python3
import argparse
import os
import sys
import time
import subprocess
import concurrent.futures
from pathlib import Path


def run_one(binary, tc_file, mode, runs, csv_time, csv_eta, timeout, use_ec):
    stem = Path(tc_file).stem
    ec_tag = "ec" if use_ec else "noec"
    tag = f"{stem}__{mode}__{ec_tag}"

    cmd = [
        binary,
        "--mode", mode,
        "--runs", str(runs),
        "--csv-time", csv_time,
        "--csv-eta", csv_eta,
        tc_file,
    ]

    if use_ec:
        cmd.append("--ec")

    t0 = time.time()
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=timeout
        )
        elapsed = time.time() - t0
        ok = (result.returncode == 0)
        stderr_line = result.stderr.strip().split("\n")[-1] if result.stderr.strip() else ""
        stdout_line = result.stdout.strip().split("\n")[-1] if result.stdout.strip() else ""
        return {
            "tag": tag,
            "ok": ok,
            "elapsed": elapsed,
            "stderr": stderr_line,
            "stdout": stdout_line,
            "returncode": result.returncode,
        }
    except subprocess.TimeoutExpired:
        return {
            "tag": tag,
            "ok": False,
            "elapsed": timeout,
            "stderr": "TIMEOUT",
            "stdout": "",
            "returncode": -1,
        }
    except Exception as e:
        return {
            "tag": tag,
            "ok": False,
            "elapsed": 0,
            "stderr": str(e),
            "stdout": "",
            "returncode": -2,
        }


def main():
    ap = argparse.ArgumentParser(description="Run simplified Dynamic BFS benchmarks")
    ap.add_argument("--binary", default="bin/benchmark")
    ap.add_argument("--test-dir", default="data/benchmark/testcases")
    ap.add_argument("--csv-time", default="results/time_vs_error.csv")
    ap.add_argument("--csv-eta", default="results/eta_scatter.csv")
    ap.add_argument("--modes", nargs="+",
                    default=["incremental", "decremental", "fullydynamic"])
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
    args = ap.parse_args()

    test_dir = Path(args.test_dir)
    if not test_dir.exists():
        print(f"ERROR: test directory '{test_dir}' does not exist.")
        sys.exit(1)

    tc_files = sorted(test_dir.glob("*.txt"))
    if args.filter:
        tc_files = [f for f in tc_files if args.filter in str(f)]

    if not tc_files:
        print(f"No .txt files found in {test_dir} (filter='{args.filter}')")
        sys.exit(1)

    os.makedirs(Path(args.csv_time).parent if Path(args.csv_time).parent.as_posix() else ".", exist_ok=True)
    os.makedirs(Path(args.csv_eta).parent if Path(args.csv_eta).parent.as_posix() else ".", exist_ok=True)

    # fresh output each run
    for p in [args.csv_time, args.csv_eta]:
        if os.path.exists(p):
            os.remove(p)

    jobs = []
    for tc in tc_files:
        modes_for_file = args.modes
        for m in ["incremental", "decremental", "fullydynamic"]:
            if f"_mode_{m}_" in tc.stem:
                modes_for_file = [m] if m in args.modes else []
                break

        for mode in modes_for_file:
            for ec_mode in args.ec_modes:
                use_ec = (ec_mode == "on")
                jobs.append((str(tc), mode, use_ec))

    print(f"Found {len(tc_files)} testcase file(s)")
    print(f"Total jobs: {len(jobs)}")
    print(f"Modes: {args.modes}")
    print(f"EC modes: {args.ec_modes}")

    ok_count = 0
    fail_count = 0

    def run_job(job):
        tc_file, mode, use_ec = job
        return run_one(
            args.binary,
            tc_file,
            mode,
            args.runs,
            args.csv_time,
            args.csv_eta,
            args.timeout,
            use_ec,
        )

    if args.parallel > 1:
        with concurrent.futures.ThreadPoolExecutor(max_workers=args.parallel) as ex:
            futures = {ex.submit(run_job, j): j for j in jobs}
            for i, fut in enumerate(concurrent.futures.as_completed(futures), 1):
                r = fut.result()
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
            status = "✓" if r["ok"] else "✗"
            print(f"[{i:3d}/{len(jobs)}] {status} {r['tag'][:90]}  {r['elapsed']:.1f}s")
            if r["stderr"]:
                print(f"       {r['stderr']}")
            if r["ok"]:
                ok_count += 1
            else:
                fail_count += 1

    print("\n" + "=" * 60)
    print(f"Done: {ok_count} succeeded, {fail_count} failed")
    print(f"time CSV: {args.csv_time}")
    print(f"eta  CSV: {args.csv_eta}")

    sys.exit(0 if fail_count == 0 else 1)


if __name__ == "__main__":
    main()