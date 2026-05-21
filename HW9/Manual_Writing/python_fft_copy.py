import csv
import matplotlib.pyplot as plt
import numpy as np

# Load sigA.csv (located one directory up) into testT and testS.
testT, testS = [], []
with open("sigA.csv") as _f:
    for _row in csv.reader(_f):
        testT.append(float(_row[0]))
        testS.append(float(_row[1]))
testT = np.array(testT)
testS = np.array(testS)

Fs = 1.0 / (testT[1] - testT[0])

# print(Fs)

# dt = 1.0 / 10000.0  # 10kHz
# t = np.arange(0.0, 1.0, dt)  # 10s
## a constant plus 100Hz and 1000Hz
# s = 4.0 * np.sin(2 * np.pi * 100 * t) + 0.25 * np.sin(2 * np.pi * 1000 * t) + 25

y = testS  # the data to make the fft from
n = len(y)  # length of the signal
k = np.arange(n)
T = n / Fs
frq = k / T  # two sides frequency range
frq = frq[range(int(n / 2))]  # one side frequency range
Y = np.fft.fft(y) / n  # fft computing and normalization
Y = Y[range(int(n / 2))]

fig, (ax1, ax2) = plt.subplots(2, 1)
ax1.plot(testT, y, "r")
ax1.set_xlabel("Time")
ax1.set_ylabel("Amplitude")
ax2.loglog(frq, abs(Y), "r")  # plotting the fft
ax2.set_xlabel("Freq (Hz)")
ax2.set_ylabel("|Y(freq)|")
plt.show()
