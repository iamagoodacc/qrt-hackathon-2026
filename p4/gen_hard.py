# Usage: python3 p4/gen_hard.py R C pearl_prob seed > puzzle.in   (always solvable; loop visits every cell)
"""Hard Masyu: winding loop = outline of a random spanning tree (visits every cell), dense pearls."""
import random, sys
def puzzle(R, C, p, seed, return_witness=False):
    rng = random.Random(seed); h, w = R // 2, C // 2
    adj = {}
    def link(a, b): adj.setdefault(a, set()).add(b); adj.setdefault(b, set()).add(a)
    def unlink(a, b): adj[a].discard(b); adj[b].discard(a)
    for i in range(h):
        for j in range(w):
            a, b, c, d = (2*i, 2*j), (2*i, 2*j+1), (2*i+1, 2*j), (2*i+1, 2*j+1)
            link(a, b); link(c, d); link(a, c); link(b, d)
    seen = {(0, 0)}; stack = [(0, 0)]
    while stack:                      # random DFS spanning tree over the 2x2 blocks
        i, j = stack[-1]
        nbrs = [(i+di, j+dj) for di, dj in ((0,1),(1,0),(0,-1),(-1,0)) if 0 <= i+di < h and 0 <= j+dj < w and (i+di, j+dj) not in seen]
        if not nbrs: stack.pop(); continue
        ni, nj = rng.choice(nbrs); seen.add((ni, nj)); stack.append((ni, nj))
        (i1, j1), (i2, j2) = sorted([(i, j), (ni, nj)])
        if i1 == i2:   # horizontal merge
            unlink((2*i1, 2*j1+1), (2*i1+1, 2*j1+1)); unlink((2*i2, 2*j2), (2*i2+1, 2*j2))
            link((2*i1, 2*j1+1), (2*i2, 2*j2)); link((2*i1+1, 2*j1+1), (2*i2+1, 2*j2))
        else:          # vertical merge
            unlink((2*i1+1, 2*j1), (2*i1+1, 2*j1+1)); unlink((2*i2, 2*j2), (2*i2, 2*j2+1))
            link((2*i1+1, 2*j1), (2*i2, 2*j2)); link((2*i1+1, 2*j1+1), (2*i2, 2*j2+1))
    straight = {v: (lambda n: n[0][0] == n[1][0] or n[0][1] == n[1][1])(sorted(adj[v])) for v in adj}
    g = [['.'] * C for _ in range(R)]
    for v, nb in adj.items():
        a, b = sorted(nb)
        if straight[v] and (not straight[a] or not straight[b]) and rng.random() < p: g[v[0]][v[1]] = 'W'
        if not straight[v] and straight[a] and straight[b] and rng.random() < p: g[v[0]][v[1]] = 'B'
    text = f"{R}\n" + "\n".join("".join(r) for r in g) + "\n"
    if not return_witness:
        return text
    directions = {(-1, 0): 1, (0, 1): 2, (1, 0): 4, (0, -1): 8}
    symbols = {5: '|', 10: '-', 3: 'L', 9: 'J', 6: 'F', 12: '7'}
    witness = [['.'] * C for _ in range(R)]
    for (r, c), neighbors in adj.items():
        mask = sum(directions[(nr-r, nc-c)] for nr, nc in neighbors)
        witness[r][c] = symbols[mask]
    return text, "\n".join("".join(row) for row in witness) + "\n"
if __name__ == '__main__':
    R, C, p, seed = int(sys.argv[1]), int(sys.argv[2]), float(sys.argv[3]), int(sys.argv[4])
    sys.stdout.write(puzzle(R, C, p, seed))
