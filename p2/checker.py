"""Toroidal word search checker; score is the statement's score divided by 100."""
import re

DIRECTIONS = [(dr, dc) for dr in (-1, 0, 1) for dc in (-1, 0, 1) if dr or dc]

class Invalid(Exception):
    pass


def parse_input(inp):
    tokens = inp.split()
    if not tokens:
        raise ValueError('empty input')
    r = int(tokens[0])
    if r < 1 or len(tokens) < r+2:
        raise ValueError('expected positive required count and forbidden count')
    required = tokens[1:1+r]
    f = int(tokens[1+r])
    forbidden = tokens[2+r:]
    if f < 0 or len(forbidden) != f:
        raise ValueError('wrong forbidden count')
    if any(re.fullmatch('[A-Z]+', w) is None for w in required+forbidden):
        raise ValueError('words must be uppercase A-Z')
    return required, forbidden


def parse_output(out):
    lines = out.splitlines()
    n = len(lines)
    if not n:
        raise Invalid('empty grid')
    grid = []
    for i, line in enumerate(lines):
        # Accept both compact local rows and HackerRank space-separated characters.
        parts = line.split()
        if len(parts) == 1 and re.fullmatch('[A-Z]+', parts[0]):
            row = parts[0]
        elif all(re.fullmatch('[A-Z]', p) for p in parts):
            row = ''.join(parts)
        else:
            raise Invalid(f'row {i+1}: expected uppercase letters')
        if len(row) != n:
            raise Invalid(f'row {i+1}: expected {n} letters')
        grid.append(row)
    return grid


def contains(grid, word):
    n = len(grid)
    for r in range(n):
        for c in range(n):
            if grid[r][c] != word[0]:
                continue
            for dr, dc in DIRECTIONS:
                if all(grid[(r+k*dr)%n][(c+k*dc)%n] == ch for k, ch in enumerate(word)):
                    return True
    return False


def score(inp, out, elapsed_s=0.0):
    required, forbidden = parse_input(inp)
    grid = parse_output(out)
    n = len(grid)
    if n < max(map(len, required)):
        raise Invalid('N smaller than longest required word')
    for word in set(required):
        if not contains(grid, word):
            raise Invalid(f'missing required word {word}')
    for word in set(forbidden):
        if contains(grid, word):
            raise Invalid(f'forbidden word {word} appears')
    return min(1.0, sum(map(len, required))/(1.7*n*n))
