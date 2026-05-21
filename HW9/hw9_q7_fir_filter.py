"""HW9 Part 7 -- FIR low-pass filter.

Applies an FIR weight array via an explicit Python double loop over
samples and weights (no scipy, no np.convolve).

A paste zone for website-generated FIR weights is provided below.
Until those are pasted, placeholder Hamming-windowed-sinc weights are
used so the script runs end-to-end.
"""

import csv
import os
import matplotlib.pyplot as plt
import numpy as np


# ============================================================
# PASTE FIR WEIGHTS FROM WEBSITE HERE
# ============================================================
# Set the value to a numpy array of taps (np.array([...])) and update
# the matching FIR_INFO string. Leave as None to use the placeholder.
FIR_WEIGHTS = {
    "sigA": None,
    "sigB": None,
    "sigC": None,
    "sigD": None,
}

FIR_INFO = {
    "sigA": "low-pass sinc, cutoff = ___ Hz, bandwidth = ___ Hz, window = ___, N = ___",
    "sigB": "low-pass sinc, cutoff = ___ Hz, bandwidth = ___ Hz, window = ___, N = ___",
    "sigC": "low-pass sinc, cutoff = ___ Hz, bandwidth = ___ Hz, window = ___, N = ___",
    "sigD": "low-pass sinc, cutoff = ___ Hz, bandwidth = ___ Hz, window = ___, N = ___",
}


# ============================================================
# PLACEHOLDER WEIGHTS -- used when FIR_WEIGHTS[name] is None.
# Hamming-windowed sinc, normalized cutoff = 0.1 * (fs/2).
# ============================================================
def _placeholder_lowpass(N, normalized_cutoff=0.1):
    n = np.arange(N)
    m = (N - 1) / 2.0
    # Sinc low-pass impulse response (causal, length N).
    h = np.sinc(normalized_cutoff * (n - m))
    # Hamming window.
    w = 0.54 - 0.46 * np.cos(2 * np.pi * n / (N - 1))
    h = h * w
    h = h / np.sum(h)
    return h


PLACEHOLDER_WEIGHTS = {
    "sigA": _placeholder_lowpass(51),
    "sigB": _placeholder_lowpass(51),
    "sigC": _placeholder_lowpass(51),
    "sigD": _placeholder_lowpass(31),
}

PLACEHOLDER_INFO = "PLACEHOLDER Hamming-windowed sinc, normalized cutoff = 0.1, N as listed"


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


def fir_filter(x, h):
    """Causal FIR filter -- explicit double loop over samples and taps."""
    n = len(x)
    N = len(h)
    y = np.zeros(n, dtype=float)
    for i in range(n):
        acc = 0.0
        for k in range(N):
            j = i - k
            if j >= 0:
                acc += h[k] * x[j]
        y[i] = acc
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


# ----------------------------------------------------------------------
# Main
# ----------------------------------------------------------------------
if __name__ == '__main__':
    here = os.path.dirname(os.path.abspath(__file__))
    out_dir = os.path.join(here, 'hw9_q7')
    os.makedirs(out_dir, exist_ok=True)
    summary = []

    for name in ('sigA', 'sigB', 'sigC', 'sigD'):
        csv_path = os.path.join(here, name + '.csv')
        t, x = load_csv(csv_path)
        Fs = sample_rate(t)

        weights = FIR_WEIGHTS.get(name)
        if weights is None:
            weights = PLACEHOLDER_WEIGHTS[name]
            info = PLACEHOLDER_INFO
            tag = 'placeholder'
        else:
            info = FIR_INFO.get(name, '')
            tag = 'website'

        N = len(weights)
        y = fir_filter(x, np.asarray(weights, dtype=float))

        title_time = '{} FIR ({}) -- N = {}, {} (Fs = {:.1f} Hz)'.format(name, tag, N, info, Fs)
        title_fft = '{} FIR FFT ({}) -- N = {}, {} (Fs = {:.1f} Hz)'.format(name, tag, N, info, Fs)

        final_path = os.path.join(out_dir, 'hw9_q7_{}_fir_final.png'.format(name))
        plot_time_compare(t, x, y, title_time, final_path)

        fft_path = os.path.join(out_dir, 'hw9_q7_{}_fir_fft.png'.format(name))
        plot_fft_compare(Fs, x, y, title_fft, fft_path)

        summary.append((name, Fs, tag, N, info, [os.path.basename(p) for p in (final_path, fft_path)]))

    print('\nHW9 Part 7 -- FIR Filter summary')
    print('-' * 60)
    for name, Fs, tag, N, info, files in summary:
        print('{}: Fs = {:8.2f} Hz, weights = {}, N = {}'.format(name, Fs, tag, N))
        print('    info: {}'.format(info))
        for fn in files:
            print('    saved {}'.format(fn))
