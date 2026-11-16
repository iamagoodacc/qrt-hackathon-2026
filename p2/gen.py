"""Known-feasible tests with reproducible random words; limits are provisional."""
import importlib.util
import random
from pathlib import Path
CONFIG = {'seed': 2201, 'random_tests': 24, 'max_words': 40, 'max_length': 12, 'max_forbidden': 20}

def generate_tests(folder):
    spec = importlib.util.spec_from_file_location('checker', Path(__file__).with_name('checker.py'))
    checker = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(checker)
    rng = random.Random(CONFIG['seed'])
    tests = [('sample', ['ACE','FIG','OAK'], []),
             ('diagram', ['PYTHON','JAVA','RUBY','PERL','RUST','SWIFT','SCALA','LUA'], []),
             ('single', ['A'], ['B']), ('duplicates', ['ABCD']*8, ['ZZ']),
             ('repeated', ['AAAA','AAA','AA'], ['B','AC']),
             ('short_forbidden', ['ABCD','BCDA','DCBA'], ['Z','YY','XX'])]
    for i in range(CONFIG['random_tests']):
        count = rng.randint(1, CONFIG['max_words'])
        req = [''.join(rng.choice('ABCDEFGHIJKLMNOPQRSTUVWX') for _ in range(rng.randint(3, CONFIG['max_length']))) for _ in range(count)]
        # A separated-row witness using Z filler guarantees feasibility; forbidden words
        # contain Y, which is absent from the entire witness.
        forbidden = []
        for _ in range(rng.randint(0, CONFIG['max_forbidden'])):
            length = rng.randint(2, CONFIG['max_length']+5)
            word = [rng.choice('ABCDEFGHIJKLMNOPQRSTUVWXYZ') for _ in range(length)]
            word[rng.randrange(length)] = 'Y'
            forbidden.append(''.join(word))
        tests.append((f'random_{i:02}', req, forbidden))
    req = [''.join(rng.choice('ABCDEFGHIJKLMNOPQRSTUVWX') for _ in range(12)) for _ in range(40)]
    tests.append(('provisional_max', req, ['Y','YY','YYYYYYYYYYYYYYYYYYYY']))
    folder.mkdir(parents=True, exist_ok=True)
    for name, req, forbidden in tests:
        text = '\n'.join([str(len(req)), *req, str(len(forbidden)), *forbidden])+'\n'
        (folder/f'{name}.in').write_text(text)
        n = max(max(map(len, req)), 2*len(req))
        for filler in 'ZYXWVUTSRQPONMLKJIHGFEDCBA':
            witness = [filler*n for _ in range(n)]
            for i, word in enumerate(req):
                witness[2*i] = word+filler*(n-len(word))
            try:
                assert checker.score(text, '\n'.join(witness)) > 0
                break
            except checker.Invalid:
                pass
        else:
            raise AssertionError('no valid witness for '+name)
    print(f'wrote {len(tests)} known-feasible tests to {folder}')

if __name__ == '__main__':
    generate_tests(Path(__file__).resolve().parent/'tests')
