"""1-D quadratic step: python3 p3/c/quad.py x1 f1 x2 f2 x3 f3 -> argmax of the parabola through the points."""
import sys
x1, f1, x2, f2, x3, f3 = map(float, sys.argv[1:7])
d = (x1 - x2) * (x1 - x3) * (x2 - x3)
A = (x3 * (f2 - f1) + x2 * (f1 - f3) + x1 * (f3 - f2)) / d
B = (x3 * x3 * (f1 - f2) + x2 * x2 * (f3 - f1) + x1 * x1 * (f2 - f3)) / d
if A >= 0: print("not concave (A=%g): move toward the best point" % A)
else: print("vertex x* = %.6g (A=%g)" % (-B / (2 * A), A))
