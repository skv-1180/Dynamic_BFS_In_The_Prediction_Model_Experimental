#!/usr/bin/env python3
import argparse
import os
import sys

import pandas as pd
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt


ALGO_COLOR = {
    "incremental": "tab:blue",
    "decremental": "tab:red",
    "fullydynamic": "tab:purple",
}

ALGO_LABEL = {
    "incremental": "Incremental",
    "decremental": "Decremental",
    "fullydynamic": "Fully Dynamic",
}

EC_STYLE = {
    "No EC": "-",
    "With EC": "--",
}

EC_MARKER = {
    "No EC": "o",
    "With EC": "s",
}


def setup_style():
    plt.rcParams.update({
        "font.size": 11,
        "axes.titlesize": 13,
        "axes.labelsize": 12,
        "legend.fontsize": 9,
        "figure.dpi": 150,
        "axes.spines.top": False,
        "axes.spines.right": False,
        "axes.grid": True,
        "grid.alpha": 0.25,
        "savefig.bbox": "tight",
    })


def save(fig, path, fmt):
    fig.tight_layout()
    out = f"{path}.{fmt}"
    fig.savefig(out, bbox_inches="tight")
    print(f"Saved: {out}")
    plt.close(fig)


def normalize_ec_value(v):
    s = str(v).strip().lower()
    if s in {"1", "true", "on", "ec", "yes"}:
        return "With EC"
    if s in {"0", "false", "off", "noec", "no"}:
        return "No EC"
    return str(v)


def load_time_csv(path):
    df = pd.read_csv(path)
    df["error_rate_r"] = df["error_rate"].round(4)
    df["speedup_vs_classical"] = df["classical_total_us"] / df["online_total_us"]

    if "ec" in df.columns:
        df["ec_label"] = df["ec"].map(normalize_ec_value)
    else:
        df["ec_label"] = "No EC"

    return df


def load_eta_csv(path):
    df = pd.read_csv(path)
    df["error_rate_r"] = df["error_rate"].round(4)

    if "ec" in df.columns:
        df["ec_label"] = df["ec"].map(normalize_ec_value)
    else:
        df["ec_label"] = "No EC"

    return df

def plot_runtime_vs_error(df, out, fmt):
    def fmt_error_rate(x):
        x = float(x)
        if abs(x - round(x)) < 1e-9:
            return f"{x:.1f}"
        if abs(x * 10 - round(x * 10)) < 1e-9:
            return f"{x:.1f}"
        return f"{x:.2f}".rstrip("0").rstrip(".")

    df = df.copy()
    df["m_total"] = df["m_init"] + df["m_updates"]

    for (n, m_total), size_df in df.groupby(["n", "m_total"]):
        fig, ax = plt.subplots(figsize=(10.5, 6.0))

        for algo in ["incremental", "decremental", "fullydynamic"]:
            algo_df = size_df[size_df["algo"] == algo]
            if algo_df.empty:
                continue

            # No EC and With EC online runtimes
            for ec_label in ["No EC", "With EC"]:
                sub = algo_df[algo_df["ec_label"] == ec_label]
                if sub.empty:
                    continue

                grp = (
                    sub.groupby("error_rate_r", as_index=False)
                    .agg({"online_total_us": "mean"})
                    .sort_values("error_rate_r")
                )

                grp["online_total_ms"] = grp["online_total_us"] / 1000.0

                ax.plot(
                    grp["error_rate_r"],
                    grp["online_total_ms"],
                    marker=EC_MARKER[ec_label],
                    linestyle=EC_STYLE[ec_label],
                    color=ALGO_COLOR[algo],
                    linewidth=2,
                    markersize=5,
                    label=f"{ALGO_LABEL[algo]} ({ec_label})"
                )

            # Classical runtime
            classical_grp = (
                algo_df.groupby("error_rate_r", as_index=False)
                .agg({"classical_total_us": "mean"})
                .sort_values("error_rate_r")
            )

            classical_grp["classical_total_ms"] = (
                classical_grp["classical_total_us"] / 1000.0
            )

            ax.plot(
                classical_grp["error_rate_r"],
                classical_grp["classical_total_ms"],
                marker="^",
                linestyle=":",
                color=ALGO_COLOR[algo],
                linewidth=2,
                markersize=5,
                label=f"{ALGO_LABEL[algo]} (Classical)"
            )

        ax.set_xlim(0.0, 1.08)

        xticks = [
            0.0, 0.1, 0.2, 0.3, 0.4,
            0.5, 0.6, 0.7, 0.8, 0.9, 1.0
        ]
        ax.set_xticks(xticks)
        ax.set_xticklabels([fmt_error_rate(x) for x in xticks])

        ax.set_ylim(bottom=0)
        ax.set_xlabel("Prediction error rate")
        ax.set_ylabel("Runtime (ms)")
        ax.set_title(f"Runtime vs Error Rate, n={n}, m={m_total}")
        ax.legend(fontsize=8, ncol=2)
        ax.grid(True, alpha=0.25)

        save(
            fig,
            f"{out}_n{n}_m{m_total}",
            fmt
        )

def main():
    ap = argparse.ArgumentParser(description="Plot simplified Dynamic BFS benchmark results with EC comparison")
    ap.add_argument("--time-csv", default="results/time_vs_error.csv")
    ap.add_argument("--output-dir", default="results/plots")
    ap.add_argument("--format", default="png", choices=["png", "pdf", "svg"])
    ap.add_argument(
        "--split-ec",
        action="store_true",
        help="Save separate plots for No EC and With EC instead of putting them in the same image"
    )
    args = ap.parse_args()

    if not os.path.exists(args.time_csv):
        print(f"ERROR: missing {args.time_csv}")
        sys.exit(1)
 

    os.makedirs(args.output_dir, exist_ok=True)
    setup_style()

    time_df = load_time_csv(args.time_csv)

    print(f"Loaded time rows: {len(time_df)}")
    print("EC labels in time CSV:", sorted(time_df["ec_label"].dropna().unique()))

    plot_runtime_vs_error(
        time_df,
        os.path.join(args.output_dir, "runtime_vs_error_rate_ec_compare"),
        args.format
    )


if __name__ == "__main__":
    main()