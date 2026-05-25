import serial
import time
import numpy as np
import matplotlib.pyplot as plt


def plotTimeAndFFT(t, raw, filtered):
    Fs = 1.0 / np.mean(np.diff(t))
    n = len(raw)
    frq = np.arange(n) * Fs / n
    frq = frq[: n // 2]
    Yraw = np.fft.fft(raw - np.mean(raw)) / n
    Yraw = Yraw[: n // 2]
    Yfilt = np.fft.fft(filtered - np.mean(filtered)) / n
    Yfilt = Yfilt[: n // 2]

    print(f"Fs = {Fs:.2f} Hz (Nyquist = {Fs / 2:.2f} Hz)")

    fig, axes = plt.subplots(2, 2, figsize=(12, 7))
    (ax_tr, ax_tf), (ax_fr, ax_ff) = axes

    ax_tr.plot(t, raw, "b-")
    ax_tr.set_xlabel("Time [s]")
    ax_tr.set_ylabel("Raw Force")
    ax_tr.set_title(f"Raw vs Time (Fs = {Fs:.2f} Hz)")

    ax_tf.plot(t, filtered, "r-")
    ax_tf.set_xlabel("Time [s]")
    ax_tf.set_ylabel("Filtered Force")
    ax_tf.set_title("Filtered vs Time")

    ax_fr.loglog(frq, abs(Yraw), "b")
    ax_fr.set_xlabel("Freq (Hz)")
    ax_fr.set_ylabel("|Y(freq)|")
    ax_fr.set_title("Raw FFT")

    ax_ff.loglog(frq, abs(Yfilt), "r")
    ax_ff.set_xlabel("Freq (Hz)")
    ax_ff.set_ylabel("|Y(freq)|")
    ax_ff.set_title("Filtered FFT")

    ax_ff.set_ylim(ax_fr.get_ylim())

    fig.tight_layout()
    plt.show()


PORT = "/dev/tty.usbmodem101"
BAUD = 115200
NUM_SAMPLES = 1500

times = []
rawData = []
filteredData = []

ser = serial.Serial(PORT, BAUD, timeout=5)

time.sleep(2)

ser.reset_input_buffer()
command = f"{NUM_SAMPLES}\n"
ser.write(command.encode())

print(f"Collecting {NUM_SAMPLES} samples...")

# for i in range(NUM_SAMPLES):
#    line = ser.readline().decode(errors="ignore").strip()
#    print(line)

# ser.close()
# exit()

i = 0
while i < NUM_SAMPLES:
    line = ser.readline().decode(errors="ignore").strip()
    parts = line.split(",")
    if len(parts) != 3:
        print(f"skipping malformed line: {line!r}", flush=True)
        continue
    try:
        t, r, f = int(parts[0]), int(parts[1]), int(parts[2])
    except ValueError:
        print(f"skipping non-integer line: {line!r}", flush=True)
        continue
    times.append(t)
    rawData.append(r)
    filteredData.append(f)
    i += 1

ser.close()

timeStamp = np.array(times) / 1000.0  # ms -> s
raw = np.array(rawData)
filtered = np.array(filteredData)

# print(timeStamp[:5])
# print(raw[:5])
# print(filtered[:5])

plotTimeAndFFT(timeStamp, raw, filtered)
