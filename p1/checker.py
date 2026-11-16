"""P1 square packing checker. Input: n followed by n positive integer sides.

Output: exactly n lines with two integer coordinates each. Score = (L / A)^20.
No coordinate sign restriction or runtime penalty is specified in the statement.
"""
import math
import re

CONFIG = {"power": 20}


class Invalid(Exception):
    pass


def parse_input(inp: str):
    tokens = inp.split()
    if not tokens:
        raise ValueError("empty input")
    n = int(tokens[0])
    if n < 1 or len(tokens) != n + 1:
        raise ValueError("expected positive n and exactly n sides")
    sides = list(map(int, tokens[1:]))
    if min(sides) <= 0:
        raise ValueError("sides must be positive")
    return sides


def parse_output(out: str, n: int):
    lines = out.splitlines()
    if len(lines) != n:
        raise Invalid(f"expected {n} lines, got {len(lines)}")
    coords = []
    for i, line in enumerate(lines, 1):
        parts = line.split()
        if len(parts) != 2 or any(re.fullmatch(r"[+-]?[0-9]+", p) is None for p in parts):
            raise Invalid(f"line {i}: expected 2 integers")
        coords.append(tuple(map(int, parts)))
    return coords


def lower_bound(sides) -> int:
    m = max(sides)
    total = sum(s * s for s in sides)
    best = m * ((total + m - 1) // m)
    # By symmetry it suffices to consider the smaller dimension <= ceil(sqrt(S)).
    # For fixed ceil(S/W), only the first W can improve the product.
    width = m
    last = math.isqrt(total) + 1
    while width <= last and best != total:
        height = max(m, (total + width - 1) // width)
        best = min(best, width * height)
        if height <= m:
            break
        width = max(width + 1, (total - 1) // (height - 1) + 1)
    return best


def score(inp: str, out: str, elapsed_s: float = 0.0) -> float:
    sides = parse_input(inp)
    coords = parse_output(out, len(sides))
    rects = [(x, y, x+s, y+s) for (x, y), s in zip(coords, sides)]
    for i, (ax, ay, ax1, ay1) in enumerate(rects):
        for j in range(i):
            bx, by, bx1, by1 = rects[j]
            if ax < bx1 and bx < ax1 and ay < by1 and by < ay1:
                raise Invalid(f"squares {j+1} and {i+1} overlap")
    height = max(r[2] for r in rects) - min(r[0] for r in rects)
    width = max(r[3] for r in rects) - min(r[1] for r in rects)
    return (lower_bound(sides) / (height * width)) ** CONFIG["power"]
