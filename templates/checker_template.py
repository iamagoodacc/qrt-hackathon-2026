"""Checker template. Copy to pN/checker.py and fill in the three TODOs.

score() must return the FRACTION (0..1) of one test's points, computed EXACTLY like
the statement's formula. Raise Invalid for ANY rule break: the real grader gives 0.

Running this file directly generates tests:  python3 pN/checker.py gen
(Put the generator here or in pN/gen.py, whichever you prefer.)
"""
import random
import sys
from pathlib import Path

CONFIG = {
    "num_tests": 20,
    "seed": 1,
    # TODO: size limits copied from the statement, e.g. "max_n": 100
}


class Invalid(Exception):
    pass


def parse_input(inp: str):
    tokens = inp.split()
    # TODO: turn tokens into the problem's data
    return tokens


def parse_output(out: str, data):
    lines = out.strip().splitlines()
    # TODO: parse; raise Invalid("line 3: expected 2 integers") on any format problem
    return lines


def score(inp: str, out: str, elapsed_s: float = 0.0) -> float:
    data = parse_input(inp)
    answer = parse_output(out, data)
    # TODO 1: check EVERY rule; raise Invalid("which rule failed, where") on the first break
    # TODO 2: compute the statement's scoring formula (include any runtime factor via elapsed_s)
    return 0.0


def generate_tests(folder: Path) -> None:
    rng = random.Random(CONFIG["seed"])
    folder.mkdir(exist_ok=True)
    for i in range(CONFIG["num_tests"]):
        # TODO 3: build an input; cover small, medium and the MAXIMUM sizes
        text = "TODO\n"
        (folder / f"{i:02d}.in").write_text(text)
    print(f"wrote {CONFIG['num_tests']} tests to {folder}")


if __name__ == "__main__" and sys.argv[1:] == ["gen"]:
    generate_tests(Path(__file__).resolve().parent / "tests")
