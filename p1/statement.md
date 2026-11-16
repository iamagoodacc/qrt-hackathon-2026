# P1: Square packing++

(Transcribed from the HackerRank screenshots `p1/statement_1.png` and `p1/statement_2.png`.
Look at the images for the diagrams.)

You are given a list of positive integers [a0, a1, ..., an], where each integer represents the
side length of a square. Your task is to arrange these squares on a two-dimensional grid such that:
1. No two squares overlap.
2. All square corners have integer coordinates.
3. All sides are parallel to the X and Y axes.
4. The bounding rectangle that contains all squares (and has horizontal and vertical sides) has the
   smallest possible area.

The coordinate system is defined as follows:
- The X-axis increases downward.
- The Y-axis increases to the right.

Your program should output a list of tuples [(X0, Y0), ..., (Xn, Yn)], where each tuple represents
the coordinates of the top-left corner of the square with side length ai.

## Examples (from the diagram)

| # | input | output | bounding area shown in diagram |
|---|-------|--------|--------------------------------|
| 1 | [1] | [(0,0)] | 1 x 1 = 1 |
| 2 | [1,2] | [(0,2),(0,0)] | 2 x 3 = 6 |
| 3 | [1,2,3] | [(2,3),(0,3),(0,0)] | 3 x 5 = 15 |
| 4 | [1,2,3,4] | [(3,4),(3,5),(0,4),(0,0)] | 5 x 7 = 35 |

(Example 4: a3=4 at (0,0), a2=3 at (0,4), a0=1 at (3,4), a1=2 at (3,5). X = row going down,
Y = column going right.)

## Goal
Implement an algorithm that:
- Generates valid coordinates for all squares satisfying the constraints.
- Minimizes the total area of the bounding rectangle.
- Returns the coordinates in the same order as the input list.

## Scoring
After submitting, the checker returns a message like:
```
Valid packing. BoundingArea=1260, LowerBound=1200, Ratio=0.9524, Score=0.3769
```

BoundingArea A - the area of the smallest axis-aligned rectangle that encloses all your squares.

LowerBound L - a provable lower bound on the bounding area of *any* valid packing. With m the
largest side and S the sum of the squared sides:
```
L = min over integers W >= m of   W x max(m, ceil(S / W))
```

Score - between 0 and 1 for each test case:
```
Score = (L / A) ^ 20
```
- A score of 1 means A = L, which proves your packing is optimal.
- The true optimum can be above L, in which case nobody can reach 1 on that test case; the test
  cases are chosen so that L is close to achievable.
- An invalid packing (overlapping squares, non-integer coordinates, wrong number of lines) scores 0.

Your total is the sum over the hidden test cases; the sample test cases are not scored.

## Tips
- Squares may be placed in any order - not necessarily by size
- The optimal bounding rectangle is often not unique in shape, but its area is
- Test your non-overlap check carefully - off-by-one errors are common

## HackerRank C++ stub (language C++23, screenshot `p1/stub.png`)
```cpp
/*
 * Complete the 'pack_squares' function below.
 *
 * The function is expected to return a 2D_INTEGER_ARRAY.
 * The function accepts INTEGER_ARRAY squares as parameter.
 */
vector<vector<int>> pack_squares(vector<int> squares) {

}
```
HackerRank's own hidden main() reads the input, calls `pack_squares`, and prints the result.
Return one `{X, Y}` pair per square, in input order. See AGENTS.md "HackerRank function stubs".

## STILL MISSING (orchestrator to add)
- [ ] Constraints: max number of squares, max side length. Until known, handle ANY size safely.
- [ ] Time limit. HackerRank's usual C++ limit is 2 s: keep the time budget at ~1.5 s until known.

## Orchestrator notes (not part of the statement)
- This is the SAME problem and scoring as the practice problem `p0`. Reuse it:
  - `p0/checker.py` already implements this exact scoring formula and overlap check.
  - `best/p0/BEST.cpp` scored ~0.82 locally (~0.77 on fresh tests). Start from it.
  - CAREFUL: p0 assumed n <= 100 and side <= 30 (`MAXS = 30` in BEST.cpp sizes its arrays).
    If the real limits are bigger, that code must be changed or it will crash / go out of bounds.
  - BEST.cpp has its own main() reading stdin. Move the logic into `pack_squares()`, reset ALL
    global state inside it (e.g. `cntInit`), start the timer inside it, remove the argv
    overrides, and put a stdin-reading main inside `#ifdef LOCAL` for the harness.
