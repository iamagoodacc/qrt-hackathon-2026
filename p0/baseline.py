"""P0 baseline: shelf packing. Valid but weak - the point of the mock is to beat it.

Sort squares biggest first, fill rows left to right up to a fixed width, start a new
row when the next square doesn't fit. Row height = the first (largest) square in it.
"""
import math
import sys

CONFIG = {
    "width_factor": 1.2,  # target width = this * sqrt(total area)
}


def main():
    tokens = sys.stdin.read().split()
    n = int(tokens[0])
    sides = [int(t) for t in tokens[1:1 + n]]
    total = sum(s * s for s in sides)
    width = max(max(sides), math.ceil(CONFIG["width_factor"] * math.sqrt(total)))

    order = sorted(range(n), key=lambda i: -sides[i])
    pos = [None] * n
    row_top, row_height, col = 0, 0, 0
    for i in order:
        s = sides[i]
        if col + s > width:          # start a new row
            row_top += row_height
            row_height, col = 0, 0
        pos[i] = (row_top, col)      # X = down, Y = right
        col += s
        row_height = max(row_height, s)

    print("\n".join(f"{x} {y}" for x, y in pos))


if __name__ == "__main__":
    main()
