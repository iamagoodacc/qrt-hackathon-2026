"""Simulated annealing skeleton (Python). Fill in initial(), score() and tweak().

Idea in one line: keep making small random changes; always keep improvements, and
early on ("hot") sometimes keep a worse one so you don't get stuck; get pickier over time.
Higher score() = better. Prints the best answer found before the time budget runs out.
"""
import math
import random
import sys
import time

CONFIG = {
    "time_budget_s": 8.0,  # about 80% of the problem's time limit
    "start_temp": 1.0,     # roughly the size of a typical score change early on
    "end_temp": 0.01,      # much smaller: only near-improvements accepted at the end
    "seed": 1,
}


def initial(data):
    """A simple VALID answer (greedy is fine)."""
    raise NotImplementedError


def score(data, state) -> float:
    """Higher is better. Must match the real scoring as closely as possible."""
    raise NotImplementedError


def tweak(data, state):
    """Return a NEW state with one small random change (move/swap one thing)."""
    raise NotImplementedError


def anneal(data):
    rng = random.Random(CONFIG["seed"])
    random.seed(CONFIG["seed"])
    start = time.perf_counter()
    cur = best = initial(data)
    cur_s = best_s = score(data, cur)
    iters = 0
    while (elapsed := time.perf_counter() - start) < CONFIG["time_budget_s"]:
        frac = elapsed / CONFIG["time_budget_s"]
        temp = CONFIG["start_temp"] * (CONFIG["end_temp"] / CONFIG["start_temp"]) ** frac
        cand = tweak(data, cur)
        cand_s = score(data, cand)
        if cand_s >= cur_s or rng.random() < math.exp((cand_s - cur_s) / temp):
            cur, cur_s = cand, cand_s
            if cur_s > best_s:
                best, best_s = cur, cur_s
        iters += 1
    print(f"iters={iters} best={best_s:.4f}", file=sys.stderr)
    return best


def main():
    data = sys.stdin.read().split()  # replace with real parsing
    best = anneal(data)
    print(best)  # replace with the required output format


if __name__ == "__main__":
    main()
