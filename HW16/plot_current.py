import serial
import matplotlib.pyplot as plt

PORT = "/dev/tty.usbmodem102"
BAUD = 115200
NSAMPLES = 400

ser = serial.Serial(PORT, BAUD, timeout=2)
ser.reset_input_buffer()
ser.write(b"a")

idx, des, act = [], [], []
while len(idx) < NSAMPLES:
    line = ser.readline().decode(errors="ignore").strip()
    parts = line.split()
    if len(parts) != 3:
        continue
    try:
        i, d, a = int(parts[0]), int(parts[1]), int(parts[2])
    except ValueError:
        continue
    idx.append(i)
    des.append(d)
    act.append(a)

ser.close()

plt.figure()
plt.plot(idx, des, label="desired")
plt.plot(idx, act, label="actual")
plt.xlabel("sample (ms)")
plt.ylabel("current (raw INA219 units)")
plt.title("PI current control | Kp = 0.8, Ki = 0.08")
plt.legend()
plt.grid(True)
plt.show()
