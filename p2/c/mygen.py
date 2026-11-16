"""p2-c private tests with REAL English words (hidden tests probably use real words).
Forbidden words: real words that are not substrings (fwd/back) of any required word."""
import random
from pathlib import Path
HERE = Path(__file__).resolve().parent
WORDS = """apple banana cherry grape lemon mango melon orange peach pear plum berry olive onion garlic pepper carrot potato tomato salad bread butter cheese honey sugar salt water coffee
tea milk juice cake cookie pizza pasta rice bean corn wheat oat barley cat dog horse cow sheep goat pig mouse rat rabbit lion tiger bear wolf fox deer moose eagle hawk owl crow raven
swan duck goose snake lizard frog toad shark whale dolphin seal otter beaver squirrel monkey zebra giraffe camel llama panda koala python java ruby perl rust swift scala lua kotlin
haskell erlang elixir fortran cobol pascal basic julia matlab delphi dart go ocaml lisp scheme prolog ada bash sql html css react angular node linux unix windows apple google
amazon market stock bond trade price value money cash bank loan credit debit asset risk hedge option future swap yield rate index fund share profit loss gain alpha beta gamma delta
theta sigma omega kappa lambda tensor vector matrix graph tree node edge path cycle loop queue stack heap array list hash table string number integer float double binary
red green blue yellow purple violet indigo black white brown silver gold copper iron steel tin lead zinc nickel carbon oxygen helium neon argon hydrogen sodium calcium
river ocean lake sea pond stream valley mountain hill forest desert island beach coast cliff canyon plain prairie meadow field garden park street road bridge tower castle
house home room door window wall floor roof stair table chair sofa bed lamp clock mirror carpet curtain kitchen garage school college office church temple palace museum
music piano guitar violin drum flute trumpet song dance opera ballet poem story novel book paper letter word sentence page chapter author reader writer artist painter
spring summer autumn winter january february march april may june july august monday friday sunday morning evening night noon dawn dusk star moon planet comet galaxy orbit
""".split()
CONFIG = {"seed": 7, "tests": 24}
def main():
    rng = random.Random(CONFIG["seed"])
    words = sorted(set(w.upper() for w in WORDS))
    d = HERE / "tests_eng"; d.mkdir(exist_ok=True)
    for i in range(CONFIG["tests"]):
        r = rng.choice([rng.randint(3, 15), rng.randint(15, 40), rng.randint(40, 80)])
        req = rng.sample(words, r)
        rest = [w for w in words if w not in req and not any(w in q or w[::-1] in q for q in req)]
        f = rng.randint(0, 15)
        forb = rng.sample(rest, min(f, len(rest)))
        (d / f"{i:02d}.in").write_text("\n".join([str(r), *req, str(len(forb)), *forb]) + "\n")
main()
