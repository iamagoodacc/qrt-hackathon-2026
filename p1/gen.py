"""Reproducible P1 tests; actual constraints still missing from statement.

Most tests sample the full provisional P0 ranges; explicit robustness cases exceed them.
"""
import random
from pathlib import Path

CONFIG = {"seed": 1701, "random_tests": 24, "provisional_n": 100, "provisional_side": 30}


def generate_tests(folder):
    folder.mkdir(parents=True, exist_ok=True)
    rng = random.Random(CONFIG["seed"])
    tests = [(f"sample_{k}", list(range(1, k+1))) for k in range(1, 5)]
    tests += [("unit_100", [1]*100), ("equal_100", [30]*100),
              ("max_mixed", [rng.randint(1, 30) for _ in range(100)]),
              ("dominant", [30]+[1]*99), ("alternating", [1,30]*50),
              ("large_single", [1000000000]),
              ("beyond_p0", [rng.randint(1, 1000) for _ in range(400)])]
    for i in range(CONFIG["random_tests"]):
        n = rng.randint(1, CONFIG["provisional_n"])
        tests.append((f"random_{i:02}", [rng.randint(1, CONFIG["provisional_side"]) for _ in range(n)]))
    for name, sides in tests:
        (folder / f"{name}.in").write_text(f"{len(sides)}\n" + "\n".join(map(str, sides)) + "\n")
    print(f"wrote {len(tests)} tests to {folder}")


if __name__ == "__main__":
    generate_tests(Path(__file__).resolve().parent / "tests")
