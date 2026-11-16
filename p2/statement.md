# P2: Word Search Generation

(Transcribed from the HackerRank screenshots `p2/statement_1.png` and `p2/statement_2.png`.
Look at the images for the diagram.)

## Problem statement

You are given two lists of words:
- **Required words** - every word in this list must appear somewhere in your grid, readable in a
  straight line in any of the 8 compass directions (horizontal, vertical, or diagonal; forwards
  or backwards).
- **Forbidden words** - none of these words may appear *anywhere* in your grid in any direction.

The grid wraps around at the edges (**torus topology**): a word may begin near one edge and
continue from the opposite edge, in any of the 8 directions (see below).

Your task is to produce the **smallest possible square grid** (side length N) that satisfies both
constraints. All cells must be filled with uppercase letters A-Z.

## Diagram: "Word Search Grid - 6x6 torus" (arrows show wrap-around)

Words: PYTHON, JAVA, RUBY, PERL, RUST, SWIFT, SCALA, LUA

```
QATSUR
YFLRUB
ILJAVA
UREUCW
GELLSS
YPNOHT
```
Verified (row, col, direction): PYTHON (5,1) leftwards wrapping; JAVA (2,2) right; RUBY (1,3)
right wrapping; PERL (5,1) up; RUST (0,5) left; SWIFT (4,4) up-right wrapping; SCALA (4,5)
up-left; LUA (4,3) up. Here S = 35, N = 6, density 0.972, score 57.19.

## Function signature
```python
def generate_grid(required_words: list[str], forbidden_words: list[str]) -> list[str]:
```

## Example

Required words:
```
ACE
FIG
OAK
```
Forbidden words:
```
-
```
(none)

Output (one valid 3x3 grid):
```
ACE
FIG
OAK
```

## Scoring
Each test case is scored by how densely your grid packs the required letters:
```
S       = total number of letters in the required words
density = S / N^2
score   = 100 x min(1, density / 1.7)
```
where N is your submitted grid size. Full marks need every cell to be used by two required words
on average, so words must share letters heavily.

Example: S = 500 letters in a 25x25 grid gives density 0.8 and score approximately 47.06; the
same words in a 20x20 grid give density 1.25 and score 73.53.

An invalid grid (missing required word, forbidden word present, non-square, non-letter cells, or N
smaller than the longest required word) scores **0**.

## HackerRank C++ stub (language C++23 IS allowed, screenshot `p2/stub.png`)
```cpp
/*
 * Complete the 'generate_grid' function below.
 *
 * The function is expected to return a 2D_CHARACTER_ARRAY.
 * The function accepts following parameters:
 *  1. STRING_ARRAY required_words
 *  2. STRING_ARRAY forbidden_words
 */
vector<vector<char>> generate_grid(vector<string> required_words, vector<string> forbidden_words) {

}
```
HackerRank's hidden main() is copied exactly into `p2/hr_main.cpp`. Build with `./hrbuild.sh`.
- Input: a line with R, then R lines (one required word each), then a line with F, then F lines
  (one forbidden word each). F can be 0.
- Output: N lines, letters separated by SINGLE SPACES, e.g. `A C E`. The checker must parse this.
Return N rows of N uppercase chars. Write it in C++. See AGENTS.md "HackerRank function stubs".

## STILL MISSING (orchestrator to add)
- [ ] Constraints: number of required/forbidden words, word lengths. Until known, handle any size.
- [ ] Time limit. HackerRank's usual C++ limit is 2 s: keep the time budget at ~1.5 s until known.
- [ ] Unclear rule: a forbidden word LONGER than N would wrap onto itself on the torus. Does that
      count as "appearing"? Until known, the checker should treat it as appearing (stricter = safer).

## Orchestrator notes (not part of the statement)
- The harness's checker must return a FRACTION: `min(1, density / 1.7)` (the 0-100 score / 100).
- HackerRank calls a function, so locally pick any stdin format (e.g. first line `R F`, then R
  required words, then F forbidden words) and put a thin `main` inside `#ifdef LOCAL` that calls
  `generate_grid` and prints one row per line. The whole file is then paste-ready.
- Useful limits: N can never be below the longest required word. Score is capped once
  density >= 1.7, so there is no gain from N smaller than ceil(sqrt(S / 1.7)) - stop there.
- Checker must search all N*N starts x 8 directions WITH wrap-around for every required and
  forbidden word.
