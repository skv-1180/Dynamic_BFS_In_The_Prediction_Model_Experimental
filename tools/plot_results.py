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


# def plot_runtime_vs_error(df, out, fmt):
#     fig, ax = plt.subplots(figsize=(10.5, 6.0))

#     for algo in ["incremental", "decremental", "fullydynamic"]:
#         sub_algo = df[df["algo"] == algo]
#         if sub_algo.empty:
#             continue

#         for ec_label in ["No EC", "With EC"]:
#             sub = sub_algo[sub_algo["ec_label"] == ec_label]
#             if sub.empty:
#                 continue

#             # One curve per graph size
#             for (n, m_updates), size_sub in sub.groupby(["n", "m_updates"]):
#                 grp = (
#                     size_sub.groupby("error_rate_r", as_index=False)
#                     .agg({
#                         "online_total_us": "mean",
#                     })
#                     .sort_values("error_rate_r")
#                 )

#                 # Convert microseconds to milliseconds
#                 grp["online_total_ms"] = grp["online_total_us"] / 1000.0

#                 ax.plot(
#                     grp["error_rate_r"],
#                     grp["online_total_ms"],
#                     marker=EC_MARKER[ec_label],
#                     linestyle=EC_STYLE[ec_label],
#                     linewidth=2,
#                     label=f"{ALGO_LABEL[algo]} ({ec_label}), n={n}, updates={m_updates}"
#                 )

#     ax.set_xlabel("Prediction error rate")
#     ax.set_ylabel("Average online runtime (ms)")
#     ax.set_title("Runtime vs Prediction Error Rate by Graph Size")
#     ax.set_ylim(bottom=0)
#     ax.legend(fontsize=7, ncol=2)
#     save(fig, out, fmt)

# def plot_runtime_vs_error(df, out, fmt):
#     """
#     For each fixed (n, m_updates), plot one graph containing:
#       1. No EC online runtime
#       2. With EC online runtime
#       3. Classical runtime
#     """

#     def fmt_error_rate(x):
#         x = float(x)
#         if abs(x - round(x)) < 1e-9:
#             return f"{x:.1f}"
#         if abs(x * 10 - round(x * 10)) < 1e-9:
#             return f"{x:.1f}"
#         return f"{x:.2f}".rstrip("0").rstrip(".")


#     for (n, m_updates), size_df in df.groupby(["n", "m_updates"]):
#         fig, ax = plt.subplots(figsize=(10.5, 6.0))

#         for algo in ["incremental", "decremental", "fullydynamic"]:
#             algo_df = size_df[size_df["algo"] == algo]
#             if algo_df.empty:
#                 continue

#             # No EC and With EC online runtimes
#             for ec_label in ["No EC", "With EC"]:
#                 sub = algo_df[algo_df["ec_label"] == ec_label]
#                 if sub.empty:
#                     continue

#                 grp = (
#                     sub.groupby("error_rate_r", as_index=False)
#                     .agg({"online_total_us": "mean"})
#                     .sort_values("error_rate_r")
#                 )

#                 grp["online_total_ms"] = grp["online_total_us"] / 1000.0

#                 ax.plot(
#                     grp["error_rate_r"],
#                     grp["online_total_ms"],
#                     marker=EC_MARKER[ec_label],
#                     linestyle=EC_STYLE[ec_label],
#                     color=ALGO_COLOR[algo],
#                     linewidth=2,
#                     markersize=5,
#                     label=f"{ALGO_LABEL[algo]} ({ec_label})"
#                 )

#             # Classical runtime
#             classical_grp = (
#                 algo_df.groupby("error_rate_r", as_index=False)
#                 .agg({"classical_total_us": "mean"})
#                 .sort_values("error_rate_r")
#             )

#             classical_grp["classical_total_ms"] = (
#                 classical_grp["classical_total_us"] / 1000.0
#             )

#             ax.plot(
#                 classical_grp["error_rate_r"],
#                 classical_grp["classical_total_ms"],
#                 marker="^",
#                 linestyle=":",
#                 color=ALGO_COLOR[algo],
#                 linewidth=2,
#                 markersize=5,
#                 label=f"{ALGO_LABEL[algo]} (Classical)"
#             )

#         ax.set_xlim(0.0, 1.08)
#         xticks = [
#             0.0, 0.1, 0.2, 0.3, 0.4,
#             0.5, 0.6, 0.7, 0.8, 0.9, 1.0
#         ]
#         ax.set_xticks(xticks)
#         ax.set_xticklabels([fmt_error_rate(x) for x in xticks])

#         ax.set_ylim(bottom=0)
#         ax.set_xlabel("Prediction error rate")
#         ax.set_ylabel("Runtime (ms)")
#         ax.set_title(f"Runtime vs Error Rate, n={n}, m={m_updates}")
#         ax.legend(fontsize=8, ncol=2)
#         ax.grid(True, alpha=0.25)

#         save(
#             fig,
#             f"{out}_n{n}_m{m_updates}",
#             fmt
#         )

def plot_runtime_vs_error(df, out, fmt):
    """
    For each fixed (n, m), where m = m_init + m_updates,
    plot one graph containing:
      1. No EC online runtime
      2. With EC online runtime
      3. Classical runtime
    """

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

# def plot_speedup_vs_error(df, out, fmt):
#     fig, ax = plt.subplots(figsize=(10.5, 6.0))

#     for algo in ["incremental", "decremental", "fullydynamic"]:
#         for ec_label in ["No EC", "With EC"]:
#             sub = df[(df["algo"] == algo) & (df["ec_label"] == ec_label)]
#             if sub.empty:
#                 continue

#             grp = (
#                 sub.groupby("error_rate_r", as_index=False)
#                 .agg({
#                     "classical_total_us": "mean",
#                     "online_total_us": "mean",
#                 })
#                 .sort_values("error_rate_r")
#             )

#             # Proper speedup: average classical runtime / average online runtime
#             grp["speedup_vs_classical"] = (
#                 grp["classical_total_us"] / grp["online_total_us"]
#             )

#             ax.plot(
#                 grp["error_rate_r"],
#                 grp["speedup_vs_classical"],
#                 marker=EC_MARKER[ec_label],
#                 linestyle=EC_STYLE[ec_label],
#                 color=ALGO_COLOR[algo],
#                 linewidth=2,
#                 markersize=5,
#                 label=f"{ALGO_LABEL[algo]} ({ec_label})"
#             )

#             # Show value at every point: x and y
#             for _, row in grp.iterrows():
#                 x = row["error_rate_r"]
#                 y = row["speedup_vs_classical"]

#                 ax.annotate(
#                     f"({x:g}, {y:.2f}x)",
#                     xy=(x, y),
#                     xytext=(0, 7),
#                     textcoords="offset points",
#                     ha="center",
#                     fontsize=7,
#                     rotation=30
#                 )

#     ax.axhline(1.0, color="black", linestyle="--", linewidth=1)
#     ax.text(
#         0.01,
#         1.05,
#         "break-even",
#         fontsize=8,
#         color="black"
#     )

#     # Normal linear scale
#     max_x = df["error_rate_r"].max()
#     ax.set_xlim(0, max_x)
#     ax.set_ylim(bottom=0)

#     xticks = [
#         0.0, 0.1, 0.2, 0.3,
#         0.4, 0.5, 0.6, 0.7, 0.8,
#         0.9, 1.0
#     ]
#     xticks = [x for x in xticks if x <= max_x]
#     ax.set_xticks(xticks)
#     ax.set_xticklabels([f"{x:g}" for x in xticks])

#     ax.set_xlabel("Prediction error rate")
#     ax.set_ylabel("Speedup over classical")
#     ax.set_title("Speedup over Classical vs Prediction Error Rate")

#     ax.grid(True, alpha=0.25)
#     ax.legend(ncol=2)

#     save(fig, out, fmt)

def plot_speedup_vs_error(df, out, fmt):
    fig, ax = plt.subplots(figsize=(10.5, 6.0))

    def fmt_error_rate(x):
        x = float(x)
        if abs(x - round(x)) < 1e-9:
            return f"{x:.1f}"
        if abs(x * 10 - round(x * 10)) < 1e-9:
            return f"{x:.1f}"
        return f"{x:.2f}".rstrip("0").rstrip(".")

    for algo in ["incremental", "decremental", "fullydynamic"]:
        for ec_label in ["No EC", "With EC"]:
            sub = df[(df["algo"] == algo) & (df["ec_label"] == ec_label)]
            if sub.empty:
                continue

            grp = (
                sub.groupby("error_rate_r", as_index=False)
                .agg({
                    "classical_total_us": "mean",
                    "online_total_us": "mean",
                })
                .sort_values("error_rate_r")
            )

            grp["speedup_vs_classical"] = (
                grp["classical_total_us"] / grp["online_total_us"]
            )

            ax.plot(
                grp["error_rate_r"],
                grp["speedup_vs_classical"],
                marker=EC_MARKER[ec_label],
                linestyle=EC_STYLE[ec_label],
                color=ALGO_COLOR[algo],
                linewidth=2,
                markersize=5,
                label=f"{ALGO_LABEL[algo]} ({ec_label})"
            )

            for _, row in grp.iterrows():
                x = float(row["error_rate_r"])
                y = row["speedup_vs_classical"]

                # Put the 1.0 label to the RIGHT of the point,
                # and give extra canvas on the right so it appears after 0.99.
                if abs(x - 1.0) < 1e-9:
                    xytext = (10, 7)
                    ha = "left"
                elif x >= 0.95:
                    xytext = (-10, 7)
                    ha = "right"
                elif abs(x - 0.0) < 1e-9:
                    xytext = (12, 7)
                    ha = "left"
                else:
                    xytext = (0, 7)
                    ha = "center"

                ax.annotate(
                    f"({fmt_error_rate(x)}, {y:.2f}x)",
                    xy=(x, y),
                    xytext=xytext,
                    textcoords="offset points",
                    ha=ha,
                    fontsize=7,
                    rotation=30,
                    clip_on=False
                )

    ax.axhline(1.0, color="black", linestyle="--", linewidth=1)
    ax.text(0.01, 1.05, "break-even", fontsize=8, color="black")

    xticks = [
        0.0, 0.1, 0.2, 0.3, 0.4,
        0.5, 0.6, 0.7, 0.8, 0.9, 1.0
    ]

    # Extra right-side width so the x=1.0 point/label comes after x=0.99 cleanly.
    ax.set_xlim(0.0, 1.08)
    ax.set_xticks(xticks)
    ax.set_xticklabels([f"{x:.1f}" for x in xticks])

    ax.set_ylim(bottom=0)
    ax.set_xlabel("Prediction error rate")
    ax.set_ylabel("Speedup over classical")
    ax.set_title("Speedup over Classical vs Prediction Error Rate")

    ax.grid(True, alpha=0.25)
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

def plot_all_for_df(time_df, output_dir, fmt, suffix=""):
    plot_runtime_vs_error(
        time_df,
        os.path.join(output_dir, f"runtime_vs_error_rate{suffix}"),
        fmt
    )

    plot_speedup_vs_error(
        time_df,
        os.path.join(output_dir, f"speedup_vs_error_rate{suffix}"),
        fmt
    )

    # Enable these if needed later:
    # plot_prediction_accuracy_vs_error(
    #     time_df,
    #     os.path.join(output_dir, f"prediction_accuracy_vs_error_rate{suffix}"),
    #     fmt
    # )

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

    # Runtime-vs-error should always compare No EC, With EC, and Classical
    # for the same fixed (n, m_updates). Do not split it by EC.
    plot_runtime_vs_error(
        time_df,
        os.path.join(args.output_dir, "runtime_vs_error_rate_ec_compare"),
        args.format
    )
    
    # if args.split_ec:
    #     for ec_label, suffix in [
    #         ("No EC", "_no_ec"),
    #         ("With EC", "_with_ec"),
    #     ]:
    #         sub_df = time_df[time_df["ec_label"] == ec_label].copy()

    #         if sub_df.empty:
    #             print(f"Skipping {ec_label}: no rows found")
    #             continue

    #         print(f"Plotting separate graphs for {ec_label}: {len(sub_df)} rows")

    #         # Do not call plot_runtime_vs_error here.
    #         plot_speedup_vs_error(
    #             sub_df,
    #             os.path.join(args.output_dir, f"speedup_vs_error_rate{suffix}"),
    #             args.format
    #         )
    # else:
    #     plot_speedup_vs_error(
    #         time_df,
    #         os.path.join(args.output_dir, "speedup_vs_error_rate_ec_compare"),
    #         args.format
    #     )
 
    print(f"All plots saved to: {args.output_dir}")


if __name__ == "__main__":
    main()