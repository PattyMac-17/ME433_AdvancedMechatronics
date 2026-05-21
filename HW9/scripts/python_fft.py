import csv
import os
import matplotlib.pyplot as plt
import numpy as np


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


def compute_fft(y, Fs):
    n = len(y)
    k = np.arange(n)
    T = n / Fs
    frq = k / T
    frq = frq[range(int(n / 2))]
    Y = np.fft.fft(y) / n
    Y = Y[range(int(n / 2))]
    return frq, np.abs(Y)


def plot_signal_and_fft(name, t, y, out_path):
    Fs = len(t) / (t[-1] - t[0])
    frq, mag = compute_fft(y, Fs)

    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(9, 6))
    ax1.plot(t, y, 'b')
    ax1.set_xlabel('Time (s)')
    ax1.set_ylabel('Amplitude')
    ax1.set_title('{} -- signal vs time (Fs = {:.2f} Hz, N = {})'.format(name, Fs, len(t)))

    ax2.loglog(frq, mag, 'b')
    ax2.set_xlabel('Frequency (Hz)')
    ax2.set_ylabel('|Y(freq)|')
    ax2.set_title('{} -- FFT magnitude (one-sided)'.format(name))

    fig.tight_layout()
    fig.savefig(out_path, dpi=120, bbox_inches='tight')
    plt.close(fig)
    return Fs


if __name__ == '__main__':
    here = os.path.dirname(os.path.abspath(__file__))
    repo_root = os.path.dirname(here)
    out_dir = os.path.join(repo_root, 'Output Images', 'hw9_q4')
    os.makedirs(out_dir, exist_ok=True)
    for name in ('sigA', 'sigB', 'sigC', 'sigD'):
        csv_path = os.path.join(repo_root, 'Datasets', name + '.csv')
        out_path = os.path.join(out_dir, 'hw9_q4_{}_time_fft.png'.format(name))
        t, y = load_csv(csv_path)
        Fs = plot_signal_and_fft(name, t, y, out_path)
        print('{}: Fs = {:.2f} Hz, N = {}, saved {}'.format(name, Fs, len(t), os.path.basename(out_path)))
