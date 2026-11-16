# P4: Masyu

(Transcribed from the HackerRank screenshots `p4/statement_1.png` ... `statement_5.png`.)

Masyu is a Nikoli logic puzzle. You are given a rectangular grid of cells, some of which contain
white or black "pearls" (circles).

Your task is to draw a single closed loop that:
- visits every pearl,
- passes through cell centres, moving only horizontally or vertically (every turn is 90 degrees),
- never crosses itself and never visits a cell twice,
- obeys the pearl rules:
  - *White pearl* - the loop passes *straight through* the cell, AND *turns* in at least one of
    the two adjacent cells along the loop.
  - *Black pearl* - the loop *turns* in the cell, AND passes *straight through* the cells
    immediately before and after.

Cells without a pearl may be visited or skipped freely. Loop cells may sit next to each other
without being connected.

Puzzles may have more than one valid loop. Any valid loop is accepted, but *shorter loops score more*.

## Input format
Plain text on stdin:
```
R
<R rows of equal length C, characters from {'.', 'W', 'B'}>
```
- `.` - empty cell
- `W` - white pearl
- `B` - black pearl

Example:
```
6
......
.W....
....B.
......
.B....
....W.
```

## Output format
R rows of length C. Each cell is one of:
- `.` (period) - not on the loop
- `-` (dash) - straight segment, horizontal (connects E and W)
- `|` (vertical bar) - straight segment, vertical (connects N and S)
- `L` - corner connecting N and E
- `J` - corner connecting N and W
- `F` - corner connecting S and E
- `7` - corner connecting S and W

A valid (and shortest, length 18) solution to the example:
```
......
F-7...
L7L-7.
.|..|.
.L-7L7
...L-J
```
The *loop length* is the number of characters other than `.` : 18 here.
(Checked by the orchestrator: 18 cells, all four pearl rules hold.)

## Scoring
Each test case scores between 0 and 100:
```
score = 100 x quality x speed
```
- *Validity.* An invalid output (wrong dimensions, illegal character, broken or multiple loops, a
  pearl rule violated) scores 0.
- *Quality.* `quality = min(1, s x L_ref / L)^2`, where L is your loop length and L_ref is the
  shortest loop length known to the organisers. On most test cases L_ref is proven to be the
  shortest possible and s = 1. On a few large ones it is only the best loop the organisers found,
  and s = 0.9: matching it earns 81%, and beating it by about 10% earns full quality. Because
  quality is squared, a loop 10% longer than L_ref keeps about 83% and one 25% longer about 64%.
- *Speed.* `speed = 0.5 + 0.5 x exp(-max(0, T - 1) / 5)`, where T is your program's runtime in
  seconds on that test case. The first second is free; after that, slower answers lose up to
  half their score, never more.

Your total is the sum over all hidden test cases. The sample test cases are not scored.

## Constraints
- 6 <= R, C <= 50 (grid is rectangular, not necessarily square)
- At least 2 pearls
- Every puzzle has at least one valid loop
- 100 hidden test cases of increasing size and difficulty
- Time limit: 10 seconds per test case (a timeout scores 0)

## Tips
- Two strategies are scored: finding a valid loop at all (hard on densely pearled puzzles), and
  finding a *short* one (hard on sparsely pearled puzzles, where many loops are valid).
- A black pearl next to the border, or next to another black pearl, often has only one possible
  orientation.
- Three white pearls in a row force the loop to cross all three perpendicular to that row.
- The single-loop rule is global: a search that never checks for prematurely closed sub-loops
  wastes most of its time.
- Runtime counts. You can trade time for loop quality, but past the first second each extra
  second costs score; plan for both fast and slow paths.

## HackerRank C++ stub (language C++23)
```cpp
/*
 * Complete the 'solve' function below.
 *
 * The function is expected to return a STRING_ARRAY.
 * The function accepts STRING_ARRAY grid as parameter.
 */
vector<string> solve(vector<string> grid) {

}
```
HackerRank's hidden main() is copied exactly into `p4/hr_main.cpp`. Build with `./hrbuild.sh`.
It reads R, then R rows with getline, and prints the returned rows one per line.

## Orchestrator notes (not part of the statement)
- The function HackerRank calls is named `solve`: don't define another function called `solve`
  with the same parameters. Strip any trailing `\r` / spaces from input rows to be safe.
- Local scoring: L_ref is unknown locally. The checker should score
  `validity x speed x min(1, L_ref/L)^2` with L_ref = a FIXED per-test reference stored in a file
  (e.g. `p4/tests/NN.ref`, the shortest loop length any of our solvers found, or a proven optimum
  from a slow exact search). Keep refs fixed while comparing solutions, or scores aren't comparable.
- Speed maths: T=1s -> 1.00, T=2s -> 0.91, T=3s -> 0.84, T=5s -> 0.72. A longer search only pays
  if it shortens the loop enough (quality is squared). Default to finishing within ~1 s, and only
  use more time on tests where the extra search clearly helps.
- Validity first: a valid loop on every test (increasing size up to 50x50, dense pearls) is worth
  far more than shortening. Never output an invalid loop; if nothing valid is found, keep searching
  (up to ~8 s) rather than print garbage.
