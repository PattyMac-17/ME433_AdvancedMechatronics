# HW10 -- IMU Tilt Maze

A small Python game for ME433 HW10. The player rolls an orange ball through
a simple maze to reach a green goal by tilting a Raspberry Pi Pico that
streams accelerometer data over USB serial.

The Pico C firmware lives one directory up (`HW10/HW10.c`, etc.) and is
unchanged by this project -- this folder contains only the Python side.

## What it does

- Opens the Pico's USB serial port and reads `x,y` tilt samples in a
  background thread.
- Uses those samples as horizontal/vertical acceleration on a ball.
- The ball collides with rectangular walls and stops when it reaches the
  goal, showing a "YOU WIN!" message.
- If the Pico is not connected, the game silently falls back to the arrow
  keys so it still runs for testing.

## Serial protocol

The Pico firmware streams one packet per line over USB serial:

| field    | value                                          |
| -------- | ---------------------------------------------- |
| port     | `/dev/tty.usbmodem101`                         |
| baud     | `115200`                                       |
| format   | `x,y\n` -- two signed floats, comma-separated  |
| range    | roughly `-1.0 .. 1.0` g                        |
| `x > 0`  | tilt right                                     |
| `x < 0`  | tilt left                                      |
| `y > 0`  | tilt down                                      |
| `y < 0`  | tilt up                                        |

`serial_reader.py` clamps both values to `[-1.0, 1.0]` and silently drops
any malformed line (wrong field count, non-numeric, etc.).

## Install

From this directory:

```bash
python3 -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

`requirements.txt` pulls in:

- `pgzero` -- the Pygame Zero game framework
- `pyserial` -- the serial port library

## Run

With the venv active:

```bash
pgzrun main.py
```

You can also press the VS Code "Run" button on `main.py` -- the
`pgzrun.go()` line at the bottom is what makes that work.

The HUD in the top-left shows the live `x,y` values and whether the input
source is `PICO` (green) or `KEYBOARD` (yellow). Press `R` at any time to
reset the ball to the start.

## Verify the Pico data in a serial monitor

Before plugging the game in, you can sanity-check the Pico stream:

```bash
python -m serial.tools.miniterm /dev/tty.usbmodem101 115200
```

You should see lines like:

```
0.001,-0.399
-0.215,0.083
0.542,0.018
```

Tilt the board and watch the numbers change. Press `Ctrl-]` to exit
miniterm. (`screen /dev/tty.usbmodem101 115200` also works on macOS.)

If `/dev/tty.usbmodem101` does not exist, check `ls /dev/tty.usbmodem*`
to find the actual device name and update the `port=` default in
`serial_reader.py` if needed.

## Homework demonstration video

A short demo should show:

1. Pico plugged in over USB. Run `pgzrun main.py`.
2. HUD shows `Input: PICO` and the live `x,y` values changing as the board
   tilts.
3. Tilt the board to roll the ball through the maze to the green goal.
   "YOU WIN!" appears.
4. Press `R` to reset.
5. Unplug the Pico (or quit and relaunch without it). The HUD now shows
   `Input: KEYBOARD`. Move the ball with the arrow keys to confirm the
   fallback works.
