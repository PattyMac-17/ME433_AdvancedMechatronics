"""HW9 Part 6 -- Single-pole IIR low-pass filter.

Implements   y[i] = A * y[i-1] + B * x[i]    with  A + B = 1
manually with a Python loop. Generates candidate plots for several
A/B pairs, then saves the final chosen plots based on
BEST_IIR_WEIGHTS below.
"""

import csv
import os
import matplotlib.pyplot as plt
import numpy as np


# ----------------------------------------------------------------------
# Choose the "best by eye" (A, B) pair for each signal here.  A + B = 1.
# ----------------------------------------------------------------------
BEST_IIR_WEIGHTS = {
    "sigA": (0.95, 0.05),
    "sigB": (0.90, 0.10),
    "sigC": (0.99, 0.01),
    "sigD": (0.80, 0.20),
}

CANDIDATE_AB = [
    (0.99, 0.01),
    (0.95, 0.05),
    (0.90, 0.10),
    (0.75, 0.25),
]


# ----------------------------------------------------------------------
# Helpers (self-contained).
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


def iir_lowpass(x, A, B):
    """new_average[i] = A * new_average[i-1] + B * signal[i] (A + B = 1)."""
    assert abs(A + B - 1.0) < 1e-9, 'IIR weights must satisfy A + B = 1'
    n = len(x)
    y = np.zeros(n, dtype=float)
    y[0] = x[0]
    for i in range(1, n):
        y[i] = A * y[i - 1] + B * x[i]
    return y


def plot_time_compare(t, x, y, title, out_path):
    fig, ax = plt.subplots(figsize=(9, 4))
    ax.plot(t, x, 'k', linewidth=0.7, label='unfiltered')
    ax.plot(t, y, 'r', linewidth=0.9, label='filtered')
    ax.set_xlabel('Time (s)')
    ax.set_ylabel('Signal')
    ax.set_title(title)
    ax.legend(loc='best')
    fig.tight_layout()
    fig.savefig(out_path, dpi=120, bbox_inches='tight')
    plt.close(fig)


def plot_fft_compare(Fs, x, y, title, out_path):
    fx, mx = compute_fft(x, Fs)
    fy, my = compute_fft(y, Fs)
    fig, ax = plt.subplots(figsize=(9, 4))
    ax.loglog(fx, mx, 'k', linewidth=0.7, label='unfiltered FFT')
    ax.loglog(fy, my, 'r', linewidth=0.9, label='filtered FFT')
    ax.set_xlabel('Frequency (Hz)')
    ax.set_ylabel('|Y(freq)|')
    ax.set_title(title)
    ax.legend(loc='best')
    fig.tight_layout()
    fig.savefig(out_path, dpi=120, bbox_inches='tight')
    plt.close(fig)


def plot_candidates_time(t, x, ab_list, name, out_path):
    n = len(ab_list)
    fig, axes = plt.subplots(n, 1, figsize=(9, 2.2 * n), sharex=True)
    if n == 1:
        axes = [axes]
    for ax, (A, B) in zip(axes, ab_list):
        y = iir_lowpass(x, A, B)
        ax.plot(t, x, 'k', linewidth=0.6, label='unfiltered')
        ax.plot(t, y, 'r', linewidth=0.9, label='A = {}, B = {}'.format(A, B))
        ax.set_ylabel('Signal')
        ax.set_title('{} -- IIR candidate A = {}, B = {}'.format(name, A, B))
        ax.legend(loc='best', fontsize=8)
    axes[-1].set_xlabel('Time (s)')
    fig.tight_layout()
    fig.savefig(out_path, dpi=120, bbox_inches='tight')
    plt.close(fig)


# ----------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------
if __name__ == '__main__':
    here = os.path.dirname(os.path.abspath(__file__))
    out_dir = os.path.join(here, 'hw9_q6')
    os.makedirs(out_dir, exist_ok=True)
    summary = []

    for name in ('sigA', 'sigB', 'sigC', 'sigD'):
        csv_path = os.path.join(here, name + '.csv')
        t, x = load_csv(csv_path)
        Fs = sample_rate(t)

        cand_path = os.path.join(out_dir, 'hw9_q6_{}_iir_candidates.png'.format(name))
        plot_candidates_time(t, x, CANDIDATE_AB, name, cand_path)

        A, B = BEST_IIR_WEIGHTS[name]
        y = iir_lowpass(x, A, B)

        final_path = os.path.join(out_dir, 'hw9_q6_{}_iir_final.png'.format(name))
        plot_time_compare(
            t, x, y,
            '{} IIR -- A = {}, B = {} (Fs = {:.1f} Hz)'.format(name, A, B, Fs),
            final_path,
        )

        fft_path = os.path.join(out_dir, 'hw9_q6_{}_iir_fft.png'.format(name))
        plot_fft_compare(
            Fs, x, y,
            '{} IIR FFT -- A = {}, B = {} (Fs = {:.1f} Hz)'.format(name, A, B, Fs),
            fft_path,
        )

        summary.append((name, Fs, (A, B), [os.path.basename(p) for p in (cand_path, final_path, fft_path)]))

    print('\nHW9 Part 6 -- IIR Filter summary')
    print('-' * 60)
    for name, Fs, (A, B), files in summary:
        print('{}: Fs = {:8.2f} Hz, A = {}, B = {}'.format(name, Fs, A, B))
        for fn in files:
            print('    saved {}'.format(fn))
