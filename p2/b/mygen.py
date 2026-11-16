"""p2-b private English tests (word list read from p2/c/mygen.py, not executed)."""
import random, re
from pathlib import Path
HERE = Path(__file__).resolve().parent
src = (HERE.parent / "c" / "mygen.py").read_text()
WORDS = re.search(r'WORDS = """(.*?)"""', src, re.S).group(1).split()
rng = random.Random(11)
words = sorted(set(w.upper() for w in WORDS))
d = HERE / "tests_eng2"; d.mkdir(exist_ok=True)
for i in range(40):
    r = rng.choice([rng.randint(5, 20), rng.randint(20, 50), rng.randint(50, 100), rng.randint(100, 150)])
    req = rng.sample(words, r)
    rest = [w for w in words if w not in req and not any(w in q or w[::-1] in q for q in req)]
    forb = rng.sample(rest, min(rng.randint(0, 20), len(rest)))
    (d / f"{i:02d}.in").write_text("\n".join([str(r), *req, str(len(forb)), *forb]) + "\n")
