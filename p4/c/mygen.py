"""p4-c private harder tests: random simply connected region of dual squares -> boundary loop,
pearls on eligible loop cells with density p (sparse or dense). Ref = that loop's length (upper bound)."""
import random, json, sys
from pathlib import Path
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import checker as c
HERE = Path(__file__).resolve().parent
CONFIG = {"seed": 11, "tests": 24}

def gen(rng, R, C, fill, dens):
    SR, SC = R - 1, C - 1
    sq = [[0] * (SC + 2) for _ in range(SR + 2)]
    g = lambda i, j: sq[i + 1][j + 1]
    def yok(i, j):
        x = [0, g(i, j+1), g(i-1, j+1), g(i-1, j), g(i-1, j-1), g(i, j-1), g(i+1, j-1), g(i+1, j), g(i+1, j+1)]
        s = 0
        for k in (1, 3, 5, 7):
            k2 = 1 if k + 2 > 8 else k + 2
            s += x[k] - x[k] * x[k+1] * x[k2]
        return s
    def diag(i, j):
        for di in (-1, 0):
            for dj in (-1, 0):
                a, b, cc, d = g(i+di, j+dj), g(i+di, j+dj+1), g(i+di+1, j+dj), g(i+di+1, j+dj+1)
                if a == d and b == cc and a != b: return True
        return False
    i0, j0 = rng.randrange(SR), rng.randrange(SC)
    sq[i0+1][j0+1] = 1; cnt = 1; target = max(2, int(fill * SR * SC))
    tries = 0
    while cnt < target and tries < 200000:
        tries += 1
        i, j = rng.randrange(SR), rng.randrange(SC)
        if g(i, j): continue
        if not (g(i-1, j) or g(i+1, j) or g(i, j-1) or g(i, j+1)): continue
        if yok(i, j) != 1: continue
        sq[i+1][j+1] = 1
        if diag(i, j): sq[i+1][j+1] = 0; continue
        cnt += 1
    out = [['.'] * C for _ in range(R)]
    ch = {5: '|', 10: '-', 3: 'L', 9: 'J', 6: 'F', 12: '7'}
    for r in range(R):
        for cc in range(C):
            nw, ne, sw, se = g(r-1, cc-1), g(r-1, cc), g(r, cc-1), g(r, cc)
            m = (nw != ne) * 1 | (ne != se) * 2 | (sw != se) * 4 | (nw != sw) * 8
            if m: out[r][cc] = ch[m]
    out = [''.join(x) for x in out]
    grid = [['.'] * C for _ in range(R)]
    for r in range(R):
        for cc in range(C):
            m = c.MASK[out[r][cc]]
            if not m: continue
            nb = [(r + di, cc + dj) for d, (di, dj) in enumerate(c.DIRECTIONS) if m & (1 << d)]
            turns = [c.MASK[out[a][b]] not in c.STRAIGHT for a, b in nb]
            if m in c.STRAIGHT and any(turns) and rng.random() < dens: grid[r][cc] = 'W'
            elif m not in c.STRAIGHT and not any(turns) and rng.random() < dens: grid[r][cc] = 'B'
    grid = [''.join(x) for x in grid]
    if sum(row.count('W') + row.count('B') for row in grid) < 2: return None
    return grid, out

def main():
    rng = random.Random(CONFIG["seed"]); d = HERE / "tests_hard"; d.mkdir(exist_ok=True)
    k = 0
    while k < CONFIG["tests"]:
        R, C = rng.randint(6, 50), rng.randint(6, 50)
        dens = rng.choice([0.05, 0.15, 0.4, 0.8])
        res = gen(rng, R, C, rng.uniform(0.15, 0.6), dens)
        if not res: continue
        grid, out = res
        L = c.validate(grid, '\n'.join(out) + '\n')
        (d / f"{k:02d}.in").write_text(f"{R}\n" + "\n".join(grid) + "\n")
        (d / f"{k:02d}.ref").write_text(json.dumps({"length": L, "s": 1.0, "dens": dens}) + "\n")
        (d / f"{k:02d}.sol").write_text("\n".join(out) + "\n")
        k += 1
main()
