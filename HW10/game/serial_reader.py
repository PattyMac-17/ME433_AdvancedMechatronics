"""
serial_reader.py

Reads IMU tilt data from the Raspberry Pi Pico over USB serial.

Serial protocol (sent by the Pico firmware in HW10/HW10.c):
    - Port:        /dev/tty.usbmodem101  (also /dev/cu.usbmodem101 on macOS)
    - Baud:        115200
    - Each packet: one line, terminated by '\n'
    - Format:      "x,y" -- two signed floats separated by a comma, no spaces
    - Units:       g (acceleration / tilt), nominally in the range -1.0 .. 1.0
    - Sign:        x > 0 -> tilt right    x < 0 -> tilt left
                   y > 0 -> tilt down     y < 0 -> tilt up

The reader runs in a background daemon thread so the pgzero game loop in
main.py never blocks on serial I/O. The game just calls get_xy() every frame
to grab the most recent (x, y) sample.

The thread also auto-reconnects: if the Pico isn't plugged in yet (or gets
unplugged mid-game), the reader keeps retrying every second so the game can
switch into "PICO" mode as soon as the port appears. Malformed packets are
silently dropped.
"""

import threading
import time

try:
    import serial
except ImportError:
    serial = None

# macOS exposes both /dev/tty.usbmodemXXX and /dev/cu.usbmodemXXX for the
# same device. The "cu." (callout) variant is the right one for streaming
# devices because the "tty." variant can block on carrier-detect during
# open(). We try the requested name first, then the cu. fallback.
RECONNECT_DELAY = 1.0  # seconds between reconnect attempts


def _clamp(v, lo=-1.0, hi=1.0):
    if v < lo:
        return lo
    if v > hi:
        return hi
    return v


class SerialReader:
    def __init__(self, port="/dev/tty.usbmodem101", baud=115200):
        # Build the list of port candidates to try, in order.
        self._candidates = [port]
        if port.startswith("/dev/tty."):
            self._candidates.append("/dev/cu." + port[len("/dev/tty."):])
        self.port_name = port  # last port we successfully opened (or tried)

        self.baud = baud
        self.connected = False
        self.error = None
        self._latest = (0.0, 0.0)
        self._lock = threading.Lock()
        self._running = True
        self._ser = None

        if serial is None:
            self.error = "pyserial is not installed"
            self._running = False
            return

        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def _try_open(self):
        """Try each candidate port. Returns an open Serial or None."""
        for candidate in self._candidates:
            try:
                s = serial.Serial(candidate, self.baud, timeout=0.1)
                self.port_name = candidate
                self.error = None
                return s
            except Exception as e:
                self.error = f"{candidate}: {e}"
        return None

    def _run(self):
        """Outer loop: connect, read until disconnect, then reconnect."""
        while self._running:
            self._ser = self._try_open()
            if self._ser is None:
                # Couldn't open any candidate -- wait and try again.
                self.connected = False
                time.sleep(RECONNECT_DELAY)
                continue

            self.connected = True
            self._read_loop()
            # _read_loop returned -> port died or we're shutting down.
            self.connected = False
            try:
                self._ser.close()
            except Exception:
                pass
            self._ser = None

    def _read_loop(self):
        while self._running:
            try:
                raw = self._ser.readline()
            except Exception as e:
                # Port disconnected mid-stream.
                self.error = str(e)
                return

            if not raw:
                continue  # timeout; just loop again

            try:
                line = raw.decode("utf-8", errors="ignore").strip()
                if not line:
                    continue
                parts = line.split(",")
                if len(parts) != 2:
                    continue
                x = _clamp(float(parts[0]))
                y = _clamp(float(parts[1]))
            except (ValueError, IndexError):
                continue  # bad packet, drop it

            with self._lock:
                self._latest = (x, y)

    def get_xy(self):
        with self._lock:
            return self._latest

    def is_connected(self):
        return self.connected

    def close(self):
        self._running = False
        if self._ser is not None:
            try:
                self._ser.close()
            except Exception:
                pass
