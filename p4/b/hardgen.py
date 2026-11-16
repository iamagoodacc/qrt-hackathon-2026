"""p4-b private hard tests: random winding loop (bump moves), pearls on qualifying cells.
Writes tests_hard/NN.in and NN.ref (witness loop length = upper bound on the optimum)."""
import random, sys
from pathlib import Path
HERE = Path(__file__).resolve().parent
rng = random.Random(int(sys.argv[1]) if len(sys.argv) > 1 else 5)
D = [(-1, 0), (0, 1), (1, 0), (0, -1)]

def make(R, C, steps):
    r0, c0 = rng.randrange(R - 1), rng.randrange(C - 1)
    loop = [(r0, c0), (r0, c0 + 1), (r0 + 1, c0 + 1), (r0 + 1, c0)]
    on = set(loop)
    for _ in range(steps):
        i = rng.randrange(len(loop)); a = loop[i]; b = loop[(i + 1) % len(loop)]
        dr, dc = b[0] - a[0], b[1] - a[1]
        pr, pc = (dc, -dr) if rng.random() < 0.5 else (-dc, dr)
        a2 = (a[0] + pr, a[1] + pc); b2 = (b[0] + pr, b[1] + pc)
        if all(0 <= x[0] < R and 0 <= x[1] < C and x not in on for x in (a2, b2)):
            loop[i + 1:i + 1] = [a2, b2]; on.add(a2); on.add(b2)
    return loop

def puzzle(R, C, steps, p):
    loop = make(R, C, steps)
    n = len(loop)
    def turn(k):
        a, b, c = loop[k - 1], loop[k], loop[(k + 1) % n]
        return (b[0] - a[0], b[1] - a[1]) != (c[0] - b[0], c[1] - b[1])
    g = [['.'] * C for _ in range(R)]
    for k in range(n):
        r, c = loop[k]
        if turn(k) and not turn(k - 1) and not turn((k + 1) % n) and rng.random() < p: g[r][c] = 'B'
        elif not turn(k) and (turn(k - 1) or turn((k + 1) % n)) and rng.random() < p: g[r][c] = 'W'
    out = [['.'] * C for _ in range(R)]
    MASKCH = {5: '|', 10: '-', 3: 'L', 9: 'J', 6: 'F', 12: '7'}
    for k in range(n):
        r, c = loop[k]; m = 0
        for (x, y) in (loop[k - 1], loop[(k + 1) % n]): m |= 1 << D.index((x - r, y - c))
        out[r][c] = MASKCH[m]
    return [''.join(row) for row in g], n, [''.join(row) for row in out]

d = HERE / 'tests_hard'; d.mkdir(exist_ok=True)
for i in range(24):
    R, C = rng.randint(8, 50), rng.randint(8, 50)
    p = rng.choice([0.15, 0.4, 0.7, 0.95])
    g, n, sol = puzzle(R, C, R * C * rng.choice([1, 3, 8]), p)
    if sum(row.count('W') + row.count('B') for row in g) < 2: g, n, sol = puzzle(R, C, R * C * 3, 0.9)
    (d / f'{i:02d}.sol').write_text('\n'.join(sol) + '\n')
    (d / f'{i:02d}.in').write_text(f'{R}\n' + '\n'.join(g) + '\n')
    (d / f'{i:02d}.ref').write_text(f'{n}\n')
