# P0 (practice): Square packing

Based on Q1 of the QRT x Imperial round (26 Oct 2026), restated for practice.

## Task
You are given n squares with integer side lengths. Place every square on an integer grid,
axis-aligned, with no two squares overlapping (touching edges is fine). Make the
bounding rectangle that encloses all squares as SMALL IN AREA as possible.

## Input
First line: n. Then n lines, one side length each.
Limits (practice): 1 <= n <= 100, 1 <= side <= 30.

Sample:
```
3
1
4
5
```

## Output
n lines `X Y` = top-left corner of each square, in input order.
X increases downward, Y increases to the right. Coordinates are integers (may be any value).

## Scoring (per test)
Let m = largest side, S = sum of squared sides,
L = min over integers W >= m of  W * max(m, ceil(S / W))   (a lower bound on the area),
A = area of your bounding rectangle.

Points = (L / A) ^ 20. Invalid output (overlap, wrong line count, non-integers) = 0.
Note how harsh the power 20 is: 5% above the bound scores about 0.38.

## Time limit
2 seconds per test (practice setting).
