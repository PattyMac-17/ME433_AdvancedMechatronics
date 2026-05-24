"""
main.py -- HW10 IMU-controlled tilt maze game (pgzero)

The player rolls an orange ball through a small maze to reach the green goal
in the bottom-right corner. The ball is controlled by tilting the Raspberry
Pi Pico, which streams IMU x,y samples over USB serial.

Serial protocol (see serial_reader.py for details):
    /dev/tty.usbmodem101 @ 115200 baud, lines of "x,y" floats in [-1, 1] g.
    x > 0 = tilt right, x < 0 = tilt left
    y > 0 = tilt down,  y < 0 = tilt up

How the Pico controls the game:
    Each frame we read the most recent (x, y) sample and use it directly as
    an acceleration on the ball. Because the protocol's y > 0 means "down",
    we can apply ay = y straight to screen-space velocity (screen y also
    grows downward) without flipping the sign.

If the Pico is not connected, the game falls back to the arrow keys so it
still runs for testing or grading.

Run with:    pgzrun main.py
Reset with:  R
"""

import pgzrun
from serial_reader import SerialReader

# ---------- pgzero window config ----------
WIDTH = 800
HEIGHT = 600
TITLE = "HW10 -- IMU Tilt Maze"

# ---------- physics tunables ----------
ACCEL = 1200.0      # px/s^2 per unit g of tilt
DAMPING = 0.985     # per-frame velocity multiplier (light friction)
BALL_RADIUS = 12
START_X, START_Y = 60, 60

# ---------- serial input ----------
reader = SerialReader()

# ---------- maze layout ----------
# Border walls so the ball cannot escape the play area, plus three
# horizontal interior walls with alternating gaps that force a serpentine
# path: right -> down -> left -> down -> right -> down -> goal.
BORDER = 10
WALL_T = 20  # interior wall thickness
walls = [
    Rect((0, 0), (WIDTH, BORDER)),                       # top border
    Rect((0, HEIGHT - BORDER), (WIDTH, BORDER)),         # bottom border
    Rect((0, 0), (BORDER, HEIGHT)),                      # left border
    Rect((WIDTH - BORDER, 0), (BORDER, HEIGHT)),         # right border
    Rect((BORDER, 150), (590, WALL_T)),                  # wall A: gap on the right
    Rect((200, 300),    (WIDTH - 200 - BORDER, WALL_T)), # wall B: gap on the left
    Rect((BORDER, 450), (590, WALL_T)),                  # wall C: gap on the right
]

# Goal region in the bottom-right pocket, reachable after passing wall C.
goal_rect = Rect((650, 510), (130, 70))

# ---------- mutable game state ----------
ball_x = START_X
ball_y = START_Y
vel_x = 0.0
vel_y = 0.0
won = False
last_source = "KEYBOARD"
last_x = 0.0
last_y = 0.0


def reset():
    global ball_x, ball_y, vel_x, vel_y, won
    ball_x, ball_y = START_X, START_Y
    vel_x, vel_y = 0.0, 0.0
    won = False


def _ball_rect_at(cx, cy):
    return Rect(
        (cx - BALL_RADIUS, cy - BALL_RADIUS),
        (BALL_RADIUS * 2, BALL_RADIUS * 2),
    )


def _resolve_axis(new_pos, axis):
    """Move the ball along one axis, then push it out of any wall it hit.

    axis is "x" or "y". Returns the resolved coordinate and the new velocity
    component (zeroed if a wall was hit).
    """
    global vel_x, vel_y
    if axis == "x":
        candidate = _ball_rect_at(new_pos, ball_y)
        v = vel_x
    else:
        candidate = _ball_rect_at(ball_x, new_pos)
        v = vel_y

    for w in walls:
        if not candidate.colliderect(w):
            continue
        # Push back to the contact edge based on which way we were moving.
        if axis == "x":
            if v > 0:
                new_pos = w.left - BALL_RADIUS
            elif v < 0:
                new_pos = w.right + BALL_RADIUS
        else:
            if v > 0:
                new_pos = w.top - BALL_RADIUS
            elif v < 0:
                new_pos = w.bottom + BALL_RADIUS
        v = 0.0
        # Update candidate so a second wall on the same axis also resolves.
        if axis == "x":
            candidate = _ball_rect_at(new_pos, ball_y)
        else:
            candidate = _ball_rect_at(ball_x, new_pos)

    return new_pos, v


def update(dt):
    global ball_x, ball_y, vel_x, vel_y, won
    global last_source, last_x, last_y

    # 1. Pick an input source.
    if reader.is_connected():
        ax_raw, ay_raw = reader.get_xy()
        last_source = "PICO"
    else:
        ax_raw = (1.0 if keyboard.right else 0.0) - (1.0 if keyboard.left else 0.0)
        ay_raw = (1.0 if keyboard.down  else 0.0) - (1.0 if keyboard.up   else 0.0)
        last_source = "KEYBOARD"
    last_x, last_y = ax_raw, ay_raw

    if won:
        return

    # 2. Integrate velocity from acceleration. y > 0 = down matches screen y.
    vel_x += ax_raw * ACCEL * dt
    vel_y += ay_raw * ACCEL * dt

    # 3. Light damping so the ball settles when the Pico is held still.
    vel_x *= DAMPING
    vel_y *= DAMPING

    # 4. Move + resolve collisions, one axis at a time so the ball can slide.
    new_x = ball_x + vel_x * dt
    new_x, vel_x = _resolve_axis(new_x, "x")
    ball_x = new_x

    new_y = ball_y + vel_y * dt
    new_y, vel_y = _resolve_axis(new_y, "y")
    ball_y = new_y

    # 5. Win check.
    if goal_rect.collidepoint(ball_x, ball_y):
        won = True


def draw():
    screen.fill((25, 25, 40))
    screen.draw.filled_rect(goal_rect, (40, 180, 70))
    for w in walls:
        screen.draw.filled_rect(w, (120, 120, 140))
    screen.draw.filled_circle((ball_x, ball_y), BALL_RADIUS, (240, 150, 40))

    # HUD: live tilt values and the active input source.
    screen.draw.text(
        f"x = {last_x:+.2f}   y = {last_y:+.2f}",
        topleft=(20, 20), fontsize=24, color="white",
    )
    screen.draw.text(
        f"Input: {last_source}",
        topleft=(20, 48), fontsize=24,
        color=("lime" if last_source == "PICO" else "yellow"),
    )
    screen.draw.text(
        "R = reset",
        topleft=(20, 76), fontsize=20, color=(180, 180, 180),
    )

    if won:
        screen.draw.text(
            "YOU WIN!",
            center=(WIDTH // 2, HEIGHT // 2 - 20),
            fontsize=80, color="white",
            owidth=1.5, ocolor="black",
        )
        screen.draw.text(
            "Press R to reset",
            center=(WIDTH // 2, HEIGHT // 2 + 40),
            fontsize=32, color="white",
        )


def on_key_down(key):
    if key == keys.R:
        reset()


pgzrun.go()
