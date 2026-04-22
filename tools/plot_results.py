#!/usr/bin/env python3
import argparse
import os
import sys

import numpy as np
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
    fig, ax = plt.subplots(figsize=(9, 5.5))

    for algo in ["incremental", "decremental", "fullydynamic"]:
        for ec_label in ["No EC", "With EC"]:
            sub = df[(df["algo"] == algo) & (df["ec_label"] == ec_label)]
            if sub.empty:
                continue

            grp = sub.groupby("error_rate_r", as_index=False).agg({
                "online_total_us": "mean"
            })

            ax.plot(
                grp["error_rate_r"],
                grp["online_total_us"],
                marker=EC_MARKER[ec_label],
                linestyle=EC_STYLE[ec_label],
                color=ALGO_COLOR[algo],
                linewidth=2,
                label=f"{ALGO_LABEL[algo]} ({ec_label})"
            )

    ax.set_xlabel("Prediction error rate")
    ax.set_ylabel("Total runtime (µs)")
    ax.set_title("Runtime vs Prediction Error Rate")
    ax.legend(ncol=2)
    save(fig, out, fmt)


def plot_speedup_vs_error(df, out, fmt):
    fig, ax = plt.subplots(figsize=(9, 5.5))

    for algo in ["incremental", "decremental", "fullydynamic"]:
        for ec_label in ["No EC", "With EC"]:
            sub = df[(df["algo"] == algo) & (df["ec_label"] == ec_label)]
            if sub.empty:
                continue

            grp = sub.groupby("error_rate_r", as_index=False)["speedup_vs_classical"].mean()

            ax.plot(
                grp["error_rate_r"],
                grp["speedup_vs_classical"],
                marker=EC_MARKER[ec_label],
                linestyle=EC_STYLE[ec_label],
                color=ALGO_COLOR[algo],
                linewidth=2,
                label=f"{ALGO_LABEL[algo]} ({ec_label})"
            )

    ax.axhline(1.0, color="black", linestyle="--", linewidth=1)
    ax.set_xlabel("Prediction error rate")
    ax.set_ylabel("Speedup over classical")
    ax.set_title("Speedup vs Prediction Error Rate")
    ax.legend(ncol=2)
    save(fig, out, fmt)


def plot_prediction_accuracy_vs_error(df, out, fmt):
    fig, ax = plt.subplots(figsize=(9, 5.5))

    for algo in ["incremental", "decremental", "fullydynamic"]:
        for ec_label in ["No EC", "With EC"]:
            sub = df[(df["algo"] == algo) & (df["ec_label"] == ec_label)]
            if sub.empty:
                continue

            grp = sub.groupby("error_rate_r", as_index=False)["prediction_accuracy"].mean()

            ax.plot(
                grp["error_rate_r"],
                grp["prediction_accuracy"],
                marker=EC_MARKER[ec_label],
                linestyle=EC_STYLE[ec_label],
                color=ALGO_COLOR[algo],
                linewidth=2,
                label=f"{ALGO_LABEL[algo]} ({ec_label})"
            )

    ax.set_xlabel("Prediction error rate")
    ax.set_ylabel("Prediction accuracy")
    ax.set_title("Prediction Accuracy vs Error Rate")
    ax.legend(ncol=2)
    save(fig, out, fmt)


def plot_ec_gain_vs_error(df, out, fmt):
    fig, ax = plt.subplots(figsize=(8.5, 5.5))

    for algo in ["incremental", "decremental", "fullydynamic"]:
        sub = df[df["algo"] == algo]
        if sub.empty:
            continue

        grp = sub.groupby(["error_rate_r", "ec_label"], as_index=False)["online_total_us"].mean()
        piv = grp.pivot(index="error_rate_r", columns="ec_label", values="online_total_us")

        if "No EC" not in piv.columns or "With EC" not in piv.columns:
            continue

        gain = piv["No EC"] / piv["With EC"]

        ax.plot(
            gain.index,
            gain.values,
            marker="o",
            color=ALGO_COLOR[algo],
            linewidth=2,
            label=ALGO_LABEL[algo]
        )

    ax.axhline(1.0, color="black", linestyle="--", linewidth=1)
    ax.set_xlabel("Prediction error rate")
    ax.set_ylabel("Runtime improvement from EC")
    ax.set_title("EC Benefit vs Prediction Error Rate")
    ax.legend()
    save(fig, out, fmt)


def plot_eta_scatter(df, algo, ycol, ylabel, out, fmt):
    sub = df[df["algo"] == algo].copy()
    if sub.empty:
        print(f"Skip {algo}: no eta data")
        return

    fig, ax = plt.subplots(figsize=(8, 5.5))

    for ec_label in ["No EC", "With EC"]:
        s = sub[sub["ec_label"] == ec_label]
        if s.empty:
            continue

        ax.scatter(
            s["eta_e"],
            s[ycol],
            s=28,
            alpha=0.45 if ec_label == "No EC" else 0.65,
            marker=EC_MARKER[ec_label],
            color=ALGO_COLOR[algo],
            edgecolors="white",
            linewidth=0.4,
            label=ec_label
        )

        if len(s) >= 2:
            z = np.polyfit(s["eta_e"], s[ycol], 1)
            p = np.poly1d(z)
            xs = np.linspace(s["eta_e"].min(), s["eta_e"].max(), 200)
            ax.plot(
                xs, p(xs),
                linestyle=EC_STYLE[ec_label],
                linewidth=2,
                color=ALGO_COLOR[algo]
            )

    ax.set_xlabel(r"$\eta_e$")
    ax.set_ylabel(ylabel)
    ax.set_title(f"{ALGO_LABEL[algo]}: {ylabel} vs " + r"$\eta_e$")
    ax.legend()
    save(fig, out, fmt)


def main():
    ap = argparse.ArgumentParser(description="Plot simplified Dynamic BFS benchmark results with EC comparison")
    ap.add_argument("--time-csv", default="results/time_vs_error.csv")
    ap.add_argument("--eta-csv", default="results/eta_scatter.csv")
    ap.add_argument("--output-dir", default="results/plots")
    ap.add_argument("--format", default="png", choices=["png", "pdf", "svg"])
    args = ap.parse_args()

    if not os.path.exists(args.time_csv):
        print(f"ERROR: missing {args.time_csv}")
        sys.exit(1)
    if not os.path.exists(args.eta_csv):
        print(f"ERROR: missing {args.eta_csv}")
        sys.exit(1)

    os.makedirs(args.output_dir, exist_ok=True)
    setup_style()

    time_df = load_time_csv(args.time_csv)
    eta_df = load_eta_csv(args.eta_csv)

    print(f"Loaded time rows: {len(time_df)}")
    print(f"Loaded eta rows : {len(eta_df)}")
    print("EC labels in time CSV:", sorted(time_df["ec_label"].dropna().unique()))
    print("EC labels in eta  CSV:", sorted(eta_df["ec_label"].dropna().unique()))

    plot_runtime_vs_error(
        time_df,
        os.path.join(args.output_dir, "runtime_vs_error_rate_ec_compare"),
        args.format
    )

    plot_speedup_vs_error(
        time_df,
        os.path.join(args.output_dir, "speedup_vs_error_rate_ec_compare"),
        args.format
    )

    plot_prediction_accuracy_vs_error(
        time_df,
        os.path.join(args.output_dir, "prediction_accuracy_vs_error_rate_ec_compare"),
        args.format
    )

    plot_ec_gain_vs_error(
        time_df,
        os.path.join(args.output_dir, "ec_gain_vs_error_rate"),
        args.format
    )

    plot_eta_scatter(
        eta_df,
        "incremental",
        "eta_v",
        r"$\eta_v$",
        os.path.join(args.output_dir, "incremental_eta_v_vs_eta_e_ec_compare"),
        args.format
    )

    plot_eta_scatter(
        eta_df,
        "decremental",
        "eta_v_star",
        r"$\eta_v^*$",
        os.path.join(args.output_dir, "decremental_eta_vstar_vs_eta_e_ec_compare"),
        args.format
    )

    plot_eta_scatter(
        eta_df,
        "fullydynamic",
        "eta_v_star",
        r"$\eta_v^*$",
        os.path.join(args.output_dir, "fullydynamic_eta_vstar_vs_eta_e_ec_compare"),
        args.format
    )

    print(f"All plots saved to: {args.output_dir}")


if __name__ == "__main__":
    main()