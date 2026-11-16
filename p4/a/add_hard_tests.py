"""Import hard cases and generate ten more with checked witness references.
Existing files do not include seeds or witnesses. Recover a compatible generator witness
by deterministic seed search; a witness need not be the exact original to certify feasibility.
"""
import importlib.util
import json
from pathlib import Path

P4 = Path(__file__).resolve().parents[1]
def module(name, file):
    spec = importlib.util.spec_from_file_location(name, file)
    result = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(result)
    return result

c = module('checker', P4/'checker.py')
gen = module('gen_hard', P4/'gen_hard.py')

def main():
    folder = P4/'tests'
    refs = json.loads((folder/'references.json').read_text())
    added = []
    for source in sorted((P4/'hard').glob('*.in')):
        text = source.read_text()
        grid = c.parse_input(text)
        r, col = len(grid), len(grid[0])
        existing_sol = source.with_suffix('.sol')
        witness = None
        seed_found = None
        if existing_sol.exists():
            witness = existing_sol.read_text()
            c.validate(grid, witness)
        else:
            # Test the pearl pattern before invoking full validator to keep seed recovery fast.
            pearls = [(i,j,ch) for i,row in enumerate(grid) for j,ch in enumerate(row) if ch!='.']
            for seed in range(10000):
                candidate, sol = gen.puzzle(r, col, 1.0, seed, return_witness=True)
                full = candidate.splitlines()[1:]
                if all(full[i][j]==ch for i,j,ch in pearls):
                    c.validate(grid, sol)
                    witness, seed_found = sol, seed
                    break
            if witness is None:
                raise RuntimeError(f'No witness recovered for {source.name}')
        added.append((source.stem, text, witness, {'imported_from':str(source.relative_to(P4)), 'witness_seed':seed_found}))
        print(f'{source.name}: recovered witness seed={seed_found}', flush=True)
    sizes = [(10,10),(12,18),(16,14),(20,20),(24,30),(30,26),(36,40),(40,44),(46,48),(50,50)]
    probabilities = [.15,.25,.4,.55,.7,.85,1.0,.15,.6,1.0]
    for i, ((r,col),prob) in enumerate(zip(sizes,probabilities)):
        seed = 44100+i
        text,witness = gen.puzzle(r,col,prob,seed,return_witness=True)
        added.append((f'hard_extra_{i:02}',text,witness,{'seed':seed,'pearl_prob':prob,'generator':'gen_hard.py'}))
    for name,text,witness,meta in added:
        grid=c.parse_input(text)
        assert sum(ch in 'WB' for row in grid for ch in row)>=2
        length=c.validate(grid,witness)
        ref={'length':length,'s':1.0,'source':'validated generator witness; not proven optimum',**meta}
        key=c.key(grid)
        if key in refs and refs[key]['length'] != length:
            raise ValueError(f'Conflicting fixed reference for {name}')
        refs[key]=ref
        (folder/f'{name}.in').write_text(text)
        (folder/f'{name}.sol').write_text(witness)
        (folder/f'{name}.ref').write_text(json.dumps(ref)+'\n')
    (folder/'references.json').write_text(json.dumps(refs,indent=2)+'\n')
    c.references.cache_clear()
    for name,text,witness,_ in added:
        assert c.score(text,witness)==1.0, name
    print(f'Added {len(added)} hard tests; total={len(list(folder.glob("*.in")))}; all witnesses validate and score 1.')

if __name__=='__main__': main()
