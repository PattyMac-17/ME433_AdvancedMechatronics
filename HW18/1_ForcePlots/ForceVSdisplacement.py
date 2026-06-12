import numpy as np
import matplotlib.pyplot as plt

wall_pos = 0.8
detent_pos = 0.0
k = 1 / (1 - wall_pos)

detent_halfWidth = 0.25
detent_height = 0.5
detent_center = 0.0

x = np.linspace(-1, 1, 2000)
f = np.zeros_like(x)

right_wall = x > wall_pos
left_wall = x < -1 * wall_pos
detent = np.abs(x) < detent_halfWidth

f[right_wall] = k * (x[right_wall] - wall_pos)
f[left_wall] = k * (x[left_wall] + wall_pos)
f[detent] = detent_height * np.sin(
    2.0 * np.pi * (x[detent] - detent_center) / (2 * detent_halfWidth)
)


fig, ax = plt.subplots(figsize=(7, 4.5), dpi=150)

ax.plot(x, f, linewidth=2.5)

ax.axvline(-wall_pos, linestyle="--", linewidth=1.2, alpha=0.8)
ax.axvline(wall_pos, linestyle="--", linewidth=1.2, alpha=0.8)

ax.axvspan(-detent_halfWidth, detent_halfWidth, alpha=0.12)

ax.set_xlim(-1.0, 1.0)
ax.set_ylim(-1.05, 1.05)

ax.set_xlabel("Normalized displacement / angle")
ax.set_ylabel("Normalized Force")
ax.set_title("Single Sine Detent Haptic Profile")

ax.set_xticks(np.linspace(-1, 1, 9))
ax.set_yticks(np.linspace(-1, 1, 9))

ax.grid(True, alpha=0.25)
ax.spines["top"].set_visible(False)
ax.spines["right"].set_visible(False)

fig.tight_layout()
plt.show()
