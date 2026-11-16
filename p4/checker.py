"""Masyu validator and fixed-reference local scorer (fraction of 100 points)."""
import hashlib
import json
import math
from functools import lru_cache
from pathlib import Path

# N,E,S,W bits. Corners follow the statement, not visual font appearance.
MASK = {'.': 0, '|': 5, '-': 10, 'L': 3, 'J': 9, 'F': 6, '7': 12}
DIRECTIONS = [(-1,0),(0,1),(1,0),(0,-1)]
STRAIGHT = {5,10}

class Invalid(Exception):
    pass


def parse_input(inp):
    lines = inp.splitlines()
    if not lines:
        raise ValueError('empty input')
    r = int(lines[0])
    grid = [line.strip() for line in lines[1:]]
    if r < 1 or len(grid) != r or not grid[0] or any(len(row) != len(grid[0]) or set(row)-set('.WB') for row in grid):
        raise ValueError('malformed input grid')
    return grid


def key(grid):
    return hashlib.sha256('\n'.join(grid).encode()).hexdigest()


def validate(grid, out):
    rows = out.splitlines()
    r, c = len(grid), len(grid[0])
    if len(rows) != r or any(len(row) != c for row in rows):
        raise Invalid('wrong output dimensions')
    if any(set(row)-MASK.keys() for row in rows):
        raise Invalid('illegal loop character')
    used = {(i,j) for i in range(r) for j in range(c) if rows[i][j] != '.'}
    if not used:
        raise Invalid('empty loop')
    neighbors = {}
    for i in range(r):
        for j in range(c):
            m = MASK[rows[i][j]]
            if grid[i][j] != '.' and not m:
                raise Invalid(f'pearl at {i},{j} not visited')
            if not m:
                continue
            adj = []
            for d, (di,dj) in enumerate(DIRECTIONS):
                if not m & (1<<d):
                    continue
                ni,nj = i+di,j+dj
                if not (0<=ni<r and 0<=nj<c) or not MASK[rows[ni][nj]] & (1<<((d+2)%4)):
                    raise Invalid(f'broken connection at {i},{j}')
                adj.append((ni,nj))
            neighbors[i,j] = adj
            turns = [MASK[rows[ni][nj]] not in STRAIGHT for ni,nj in adj]
            if grid[i][j] == 'W' and (m not in STRAIGHT or not any(turns)):
                raise Invalid(f'white rule at {i},{j}')
            if grid[i][j] == 'B' and (m in STRAIGHT or any(turns)):
                raise Invalid(f'black rule at {i},{j}')
    reached = set()
    stack = [next(iter(used))]
    while stack:
        cell = stack.pop()
        if cell in reached:
            continue
        reached.add(cell)
        stack.extend(neighbors[cell])
    if reached != used:
        raise Invalid('multiple loops')
    return len(used)


@lru_cache(maxsize=1)
def references():
    path = Path(__file__).resolve().parent/'tests'/'references.json'
    return json.loads(path.read_text())


def score(inp, out, elapsed_s=0.0):
    grid = parse_input(inp)
    length = validate(grid, out)
    ref = references().get(key(grid))
    if ref is None:
        raise ValueError('no fixed local reference for this puzzle; use validate() for validity')
    quality = min(1.0, ref.get('s',1.0)*ref['length']/length)**2
    speed = 0.5+0.5*math.exp(-max(0.0,elapsed_s-1.0)/5.0)
    return quality*speed
