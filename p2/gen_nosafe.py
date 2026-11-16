"""Extra p2 tests where forbidden words cover ALL 26 letters (no 'safe' filler letter exists),
like the hidden HackerRank tests. Added by the orchestrator after 'Forbidden word ARE found'."""
import random
from pathlib import Path

CONFIG = {"seed": 4242, "cases": [(40, 3, 9, 60), (120, 4, 11, 150), (250, 4, 12, 300), (80, 5, 10, 400)]}
A = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
COMMON = ["ARE", "THE", "AND", "FOR", "YOU", "NOT", "BUT", "HIS", "ONE", "ALL", "CAN", "HER", "WAS",
          "OUT", "GET", "HAS", "HIM", "HOW", "MAN", "NEW", "NOW", "OLD", "SEE", "TWO", "WAY", "WHO",
          "BOY", "DID", "ITS", "LET", "PUT", "SAY", "SHE", "TOO", "USE", "JAZ", "QUO", "ZAX", "KEX"]

def word(rng, lo, hi):
    v, c = "AEIOUY", "BCDFGHJKLMNPQRSTVWXZ"
    return "".join(rng.choice(c if i % 2 == 0 else v) for i in range(rng.randint(lo, hi)))

def main():
    rng = random.Random(CONFIG["seed"])
    folder = Path(__file__).resolve().parent / "tests"
    for idx, (R, lo, hi, Fwant) in enumerate(CONFIG["cases"]):
        req = sorted({word(rng, lo, hi) for _ in range(R)})
        bad = lambda f: any(f in w or f[::-1] in w for w in req)
        forb = [f for f in COMMON if not bad(f)]
        while len(forb) < Fwant or set("".join(forb)) != set(A):
            f = "".join(rng.choice(A) for _ in range(rng.randint(2, 4)))
            if not bad(f) and f not in forb:
                forb.append(f)
        assert set("".join(forb)) == set(A)
        (folder / f"nosafe_{idx}.in").write_text("\n".join([str(len(req))] + req + [str(len(forb))] + forb) + "\n")
        print(f"nosafe_{idx}: R={len(req)} F={len(forb)} letters covered by forbidden = {len(set(''.join(forb)))}")

if __name__ == "__main__":
    main()
