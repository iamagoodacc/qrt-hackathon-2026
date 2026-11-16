# P0 builder notes

- Place every input square axis-aligned on integer coordinates, in input order.
- Output exactly n lines, each containing X Y; negative coordinates are allowed.
- Square interiors must be disjoint; touching edges and corners is allowed.
- Minimize the area A of the bounding rectangle, measured from minimum coordinates to maximum square extents.
- Score is (L/A)^20, where m=max(side), S=sum(side^2), and L=min over integer W>=m of W*max(m,ceil(S/W)). Invalid output scores zero.

Limits: 1<=n<=100, 1<=side<=30. Time limit: 2 seconds per test.
For computing L it suffices to check m<=W<=ceil(S/m): larger widths have height m and strictly increasing area.

Builder scope: audit existing checker, add deterministic edge-case tests, verify the checker, and save a simple C++17 descending-size shelf baseline. Stop after baseline; no optimization.
