"""Reproduce the original 20 cases and add deterministic boundary cases."""
import random
from pathlib import Path
from checker import generate_tests

CONFIG = {"seed": 731, "max_n": 100, "max_side": 30}


def main():
    folder = Path(__file__).resolve().parent / "tests"
    generate_tests(folder)
    rng = random.Random(CONFIG["seed"])
    cases = {
        "edge_sample": [1, 4, 5],
        "edge_single_min": [1],
        "edge_single_max": [30],
        "edge_touch_pair": [30, 30],
        "edge_max_ones": [1] * 100,
        "edge_max_thirties": [30] * 100,
        "edge_one_large": [30] + [1] * 99,
        "edge_alternating": [1, 30] * 50,
        "edge_staircase": list(range(1, 31)),
        "edge_near_equal": [rng.choice([28, 29, 30]) for _ in range(100)],
        "edge_max_random": [rng.randint(1, 30) for _ in range(100)],
        "edge_medium": [rng.randint(1, 30) for _ in range(50)],
    }
    for name, sides in cases.items():
        (folder / (name + ".in")).write_text(
            str(len(sides)) + "\n" + "\n".join(map(str, sides)) + "\n"
        )
    print(f"added {len(cases)} boundary cases")


if __name__ == "__main__":
    main()
