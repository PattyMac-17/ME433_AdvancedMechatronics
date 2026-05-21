"""HW9 Part 5 -- Moving average (boxcar) low-pass filter.

Implements the moving-average filter manually with a Python loop (no
np.convolve). Generates candidate plots for several window sizes, then
saves the final chosen plots based on BEST_MAF_WINDOWS below.
"""

import csv
import os
import matplotlib.pyplot as plt
import numpy as np

# ----------------------------------------------------------------------
# Choose the "best by eye" moving-average window for each signal here.
# ----------------------------------------------------------------------
BEST_MAF_WINDOWS = {
    "sigA": 50,
    "sigB": 25,
    "sigC": 100,
    "sigD": 10,
}

CANDIDATE_WINDOWS = [5, 25, 100, 500]


# ----------------------------------------------------------------------
# Helpers (self-contained so this script is independently runnable).
# ----------------------------------------------------------------------
def load_csv(path):
    t = []
    y = []
    with open(path) as f:
        reader = csv.reader(f)
        for row in reader:
            if not row:
                continue
            t.append(float(row[0]))
            y.append(float(row[1]))
    return np.array(t), np.array(y)


def sample_rate(t):
    return len(t) / (t[-1] - t[0])


def compute_fft(y, Fs):
    n = len(y)
    k = np.arange(n)
    T = n / Fs
    frq = k / T
    frq = frq[range(int(n / 2))]
    Y = np.fft.fft(y) / n
    Y = Y[range(int(n / 2))]
    return frq, np.abs(Y)


def moving_average(x, k):
    """Causal moving average using an explicit Python loop with a
    running sum. No np.convolve."""
    n = len(x)
    y = np.zeros(n, dtype=float)
    s = 0.0
    for i in range(n):
        s += x[i]
        if i >= k:
            s -= x[i - k]
            y[i] = s / k
        else:
            y[i] = s / k
    return y


def plot_time_compare(t, x, y, title, out_path):
    fig, ax = plt.subplots(figsize=(9, 4))
    ax.plot(t, x, "k", linewidth=0.7, label="unfiltered")
    ax.plot(t, y, "r", linewidth=0.9, label="filtered")
    ax.set_xlabel("Time (s)")
    ax.set_ylabel("Signal")
    ax.set_title(title)
    ax.legend(loc="best")
    fig.tight_layout()
    fig.savefig(out_path, dpi=120, bbox_inches="tight")
    plt.close(fig)


def plot_fft_compare(Fs, x, y, title, out_path):
    fx, mx = compute_fft(x, Fs)
    fy, my = compute_fft(y, Fs)
    fig, ax = plt.subplots(figsize=(9, 4))
    ax.loglog(fx, mx, "k", linewidth=0.7, label="unfiltered FFT")
    ax.loglog(fy, my, "r", linewidth=0.9, label="filtered FFT")
    ax.set_xlabel("Frequency (Hz)")
    ax.set_ylabel("|Y(freq)|")
    ax.set_title(title)
    ax.legend(loc="best")
    fig.tight_layout()
    fig.savefig(out_path, dpi=120, bbox_inches="tight")
    plt.close(fig)


def plot_candidates_time(t, x, windows, name, out_path):
    n = len(windows)
    fig, axes = plt.subplots(n, 1, figsize=(9, 2.2 * n), sharex=True)
    if n == 1:
        axes = [axes]
    for ax, k in zip(axes, windows):
        y = moving_average(x, k)
        ax.plot(t, x, "k", linewidth=0.6, label="unfiltered")
        ax.plot(t, y, "r", linewidth=0.9, label="X = {}".format(k))
        ax.set_ylabel("Signal")
        ax.set_title("{} -- MAF candidate X = {}".format(name, k))
        ax.legend(loc="best", fontsize=8)
    axes[-1].set_xlabel("Time (s)")
    fig.tight_layout()
    fig.savefig(out_path, dpi=120, bbox_inches="tight")
    plt.close(fig)


# ----------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------
if __name__ == "__main__":
    here = os.path.dirname(os.path.abspath(__file__))
    out_dir = os.path.join(here, "hw9_q5")
    cand_dir = os.path.join(here, "candidates")
    os.makedirs(out_dir, exist_ok=True)
    os.makedirs(cand_dir, exist_ok=True)
    summary = []

    for name in ("sigA", "sigB", "sigC", "sigD"):
        csv_path = os.path.join(here, name + ".csv")
        t, x = load_csv(csv_path)
        Fs = sample_rate(t)

        # Candidate sweep (time domain).
        cand_path = os.path.join(cand_dir, "hw9_q5_{}_maf_candidates.png".format(name))
        plot_candidates_time(t, x, CANDIDATE_WINDOWS, name, cand_path)

        # Chosen best window.
        k_best = BEST_MAF_WINDOWS[name]
        y = moving_average(x, k_best)

        final_path = os.path.join(out_dir, "hw9_q5_{}_maf.png".format(name))
        plot_time_compare(
            t,
            x,
            y,
            "{} MAF -- X = {} averaged points (Fs = {:.1f} Hz)".format(
                name, k_best, Fs
            ),
            final_path,
        )

        fft_path = os.path.join(out_dir, "hw9_q5_{}_maf_fft.png".format(name))
        plot_fft_compare(
            Fs,
            x,
            y,
            "{} MAF FFT -- X = {} (Fs = {:.1f} Hz)".format(name, k_best, Fs),
            fft_path,
        )

        summary.append(
            (
                name,
                Fs,
                k_best,
                [os.path.basename(p) for p in (cand_path, final_path, fft_path)],
            )
        )

    print("\nHW9 Part 5 -- Moving Average summary")
    print("-" * 60)
    for name, Fs, k_best, files in summary:
        print("{}: Fs = {:8.2f} Hz, best X = {}".format(name, Fs, k_best))
        for fn in files:
            print("    saved {}".format(fn))
