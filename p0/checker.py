"""Checker + test generator for P0 (square packing practice problem).

Score a solution:   python3 harness.py run p0 "python3 p0/baseline.py" --time-limit 2
Generate tests:     python3 p0/checker.py gen
"""
import math
import random
import sys
from pathlib import Path

CONFIG = {
    "num_tests": 20,
    "seed": 1,
    "min_n": 5,
    "max_n": 100,
    "max_side": 30,
    "power": 20,
}


class Invalid(Exception):
    pass


def parse_input(inp: str):
    tokens = inp.split()
    n = int(tokens[0])
    sides = [int(t) for t in tokens[1:1 + n]]
    if len(sides) != n:
        raise ValueError("input has fewer sides than n")
    return sides


def parse_output(out: str, n: int):
    lines = [ln for ln in out.strip().splitlines() if ln.strip()]
    if len(lines) != n:
        raise Invalid(f"expected {n} lines, got {len(lines)}")
    coords = []
    for i, ln in enumerate(lines, 1):
        parts = ln.split()
        if len(parts) != 2:
            raise Invalid(f"line {i}: expected 2 integers, got {ln!r}")
        try:
            coords.append((int(parts[0]), int(parts[1])))
        except ValueError:
            raise Invalid(f"line {i}: not integers: {ln!r}")
    return coords


def lower_bound(sides) -> int:
    m = max(sides)
    total = sum(s * s for s in sides)
    best = None
    for width in range(m, max(m, math.ceil(total / m)) + 1):
        area = width * max(m, math.ceil(total / width))
        best = area if best is None else min(best, area)
    return best


def score(inp: str, out: str, elapsed_s: float = 0.0) -> float:
    sides = parse_input(inp)
    coords = parse_output(out, len(sides))
    rects = [(x, y, x + s, y + s) for (x, y), s in zip(coords, sides)]
    for i in range(len(rects)):
        ax0, ay0, ax1, ay1 = rects[i]
        for j in range(i + 1, len(rects)):
            bx0, by0, bx1, by1 = rects[j]
            if ax0 < bx1 and bx0 < ax1 and ay0 < by1 and by0 < ay1:
                raise Invalid(f"squares {i + 1} and {j + 1} overlap")
    height = max(r[2] for r in rects) - min(r[0] for r in rects)
    width = max(r[3] for r in rects) - min(r[1] for r in rects)
    area = height * width
    return (lower_bound(sides) / area) ** CONFIG["power"]


def generate_tests(folder: Path) -> None:
    rng = random.Random(CONFIG["seed"])
    folder.mkdir(exist_ok=True)
    for i in range(CONFIG["num_tests"]):
        # sizes grow with the test index, the last ones at the maximum
        frac = i / (CONFIG["num_tests"] - 1)
        n = round(CONFIG["min_n"] + frac * (CONFIG["max_n"] - CONFIG["min_n"]))
        max_side = rng.randint(5, CONFIG["max_side"])
        sides = [rng.randint(1, max_side) for _ in range(n)]
        (folder / f"{i:02d}.in").write_text(f"{n}\n" + "\n".join(map(str, sides)) + "\n")
    print(f"wrote {CONFIG['num_tests']} tests to {folder}")


if __name__ == "__main__" and sys.argv[1:] == ["gen"]:
    generate_tests(Path(__file__).resolve().parent / "tests")
