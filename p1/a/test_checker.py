import importlib.util
import math
import random
from pathlib import Path

spec = importlib.util.spec_from_file_location("checker", Path(__file__).resolve().parents[1] / "checker.py")
c = importlib.util.module_from_spec(spec)
spec.loader.exec_module(c)

samples = [([1], "0 0\n", 1), ([1,2], "0 2\n0 0\n", 6),
           ([1,2,3], "2 3\n0 3\n0 0\n", 15),
           ([1,2,3,4], "3 4\n3 5\n0 4\n0 0\n", 35)]
for sides, out, area in samples:
    inp = str(len(sides)) + "\n" + " ".join(map(str, sides))
    expected = (c.lower_bound(sides)/area)**20
    assert math.isclose(c.score(inp, out), expected)
    shifted = "\n".join(f"{int(line.split()[0])-100} {int(line.split()[1])+17}" for line in out.splitlines())
    assert c.score(inp, shifted) == c.score(inp, out)
for out in ["0 0\n0 0\n", "0 0\n", "0 0\n0 2\n0 3\n", "0 0\n0.0 2\n", "0 0\n0 2 3\n", "0 0\n\n", "0 0\n0 2\n\n"]:
    try:
        c.score("2\n1\n2\n", out)
    except c.Invalid:
        pass
    else:
        raise AssertionError(repr(out))
assert c.score("2\n1\n1\n", "0 0\n1 0\n") == 1
assert c.score("1\n1000000000\n", "-2000000000 2000000000\n") == 1
rng = random.Random(991)
for _ in range(2000):
    sides = [rng.randint(1,50) for _ in range(rng.randint(1,30))]
    m, total = max(sides), sum(s*s for s in sides)
    brute = min(w*max(m,(total+w-1)//w) for w in range(m,(total+m-1)//m+1))
    assert c.lower_bound(sides) == brute, (sides, brute, c.lower_bound(sides))
print("checker: four samples, translations, malformed/overlap cases, large integers, 2000 exact-bound comparisons passed")
