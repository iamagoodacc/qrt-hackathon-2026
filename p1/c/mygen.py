"""p1-c private test sets (NOT the shared tests). 
tests_perfect: random W x H rectangle tiled by squares (lowest-gap random fill) -> A = L achievable.
tests_rand: fresh uniform random tests like p1/gen.py (n 1..100, side 1..maxside)."""
import random
from pathlib import Path
HERE = Path(__file__).resolve().parent
CONFIG = {"seed": 99, "perfect": 24, "rand": 24}

def perfect(rng):
    W = rng.randint(15, 120); H = rng.randint(15, 120); mx = rng.randint(8, 30)
    sky = [0] * W; sides = []
    while True:
        h0 = min(sky)
        if h0 >= H: break
        x = sky.index(h0); e = x
        while e + 1 < W and sky[e + 1] == h0: e += 1
        g = e - x + 1
        cap = min(g, H - h0, mx)
        s = rng.randint(max(1, cap // 3), cap) if rng.random() < 0.8 else rng.randint(1, cap)
        for i in range(x, x + s): sky[i] += s
        sides.append(s)
    rng.shuffle(sides)
    return sides

def main():
    rng = random.Random(CONFIG["seed"])
    for name, k, f in (("tests_perfect", CONFIG["perfect"], perfect),
                       ("tests_rand", CONFIG["rand"], lambda r: [r.randint(1, mx) for mx in [r.choice([10, 20, 30])] for _ in range(r.randint(5, 100))])):
        d = HERE / name; d.mkdir(exist_ok=True)
        for i in range(k):
            s = f(rng)
            (d / f"{i:02d}.in").write_text(f"{len(s)}\n" + "\n".join(map(str, s)) + "\n")
main()
