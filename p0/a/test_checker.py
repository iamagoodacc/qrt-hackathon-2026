"""Independent checker assertions, including statement sample and malformed output."""
import importlib.util
from pathlib import Path

spec = importlib.util.spec_from_file_location("checker", Path(__file__).resolve().parents[1] / "checker.py")
checker = importlib.util.module_from_spec(spec)
spec.loader.exec_module(checker)


def invalid(inp, out):
    try:
        checker.score(inp, out)
    except checker.Invalid:
        return
    raise AssertionError(repr(out))


sample = "3\n1\n4\n5\n"
# A=45, S=42, m=5, L=42 (6 by 7). No sample output is provided.
assert abs(checker.score(sample, "0 4\n0 0\n4 0\n") - (42 / 45) ** 20) < 1e-15
assert checker.score("1\n30\n", "-999999999999999999999 55\n") == 1
assert checker.score("2\n1\n1\n", "0 0\n0 1\n") == 1
assert checker.score("2\n1\n1\n", "0 0\n1 1\n") == (2 / 4) ** 20
for out in ["0 0\n0 0\n", "0 0\n", "0 0\n0 1\n2 2\n",
            "0 0\n\n0 1\n", "0 0\n0 1\n\n", "0 0\n0.0 1\n",
            "0 0\n1_0 1\n", "0 0\n0 1 extra\n"]:
    invalid("2\n1\n1\n", out)
invalid(sample, "0 0\n0 0\n0 0\n")
# Independently search a wider finite domain to check the lower-bound cutoff.
for a in range(1, 8):
    for b in range(1, 8):
        sides = [a, b]
        total = a*a + b*b
        m = max(sides)
        expected = min(w * max(m, (total+w-1)//w) for w in range(m, total+1))
        assert checker.lower_bound(sides) == expected
print("checker assertions passed")
