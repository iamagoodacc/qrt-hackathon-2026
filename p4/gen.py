"""Deterministic solvable Masyu tests. Fixed witness lengths are local proxies."""
import importlib.util
import json
import random
from pathlib import Path

CONFIG = {'seed':4401, 'random_tests':24}
spec = importlib.util.spec_from_file_location('checker', Path(__file__).with_name('checker.py'))
c = importlib.util.module_from_spec(spec)
spec.loader.exec_module(c)

def make_loop(r, col, vertices):
    path=[]
    for (a,b),(x,y) in zip(vertices,vertices[1:]+vertices[:1]):
        dr=(x>a)-(x<a); dc=(y>b)-(y<b)
        while (a,b)!=(x,y):
            path.append((a,b)); a+=dr; b+=dc
    assert len(set(path))==len(path)
    result=[['.']*col for _ in range(r)]
    for i,(a,b) in enumerate(path):
        mask=0
        for x,y in (path[i-1],path[(i+1)%len(path)]):
            mask |= 1<<c.DIRECTIONS.index((x-a,y-b))
        result[a][b]=next(ch for ch,m in c.MASK.items() if m==mask)
    return path,[''.join(row) for row in result]

def generate_tests(folder):
    rng=random.Random(CONFIG['seed']); folder.mkdir(parents=True,exist_ok=True)
    sample=['......','.W....','....B.','......','.B....','....W.']
    sample_out=['......','F-7...','L7L-7.','.|..|.','.L-7L7','...L-J']
    tests=[('sample',sample,sample_out)]
    for idx in range(CONFIG['random_tests']+2):
        r,col = (50,50) if idx==24 else ((6,50) if idx==25 else (rng.randint(6,50),rng.randint(6,50)))
        top=rng.randint(0,r-5); bottom=rng.randint(top+4,r-1)
        left=rng.randint(0,col-5); right=rng.randint(left+4,col-1)
        vertices=[(top,left),(top,right),(bottom,right),(bottom,left)]
        # Half are indented polygons, so rectangles alone cannot solve the suite.
        if idx%2 and right-left>=6 and bottom-top>=4:
            a=left+2; b=right-2; depth=2
            vertices=[(top,left),(top,a),(top+depth,a),(top+depth,b),(top,b),(top,right),(bottom,right),(bottom,left)]
        path,out=make_loop(r,col,vertices)
        grid=[['.']*col for _ in range(r)]; candidates=[]
        for i,(a,b) in enumerate(path):
            m=c.MASK[out[a][b]]
            prev=path[i-1]; nex=path[(i+1)%len(path)]
            turns=[c.MASK[out[x][y]] not in c.STRAIGHT for x,y in (prev,nex)]
            if m in c.STRAIGHT and any(turns): candidates.append((a,b,'W'))
            elif m not in c.STRAIGHT and not any(turns): candidates.append((a,b,'B'))
        rng.shuffle(candidates)
        count=rng.randint(2,len(candidates))
        for a,b,ch in candidates[:count]: grid[a][b]=ch
        tests.append((f'{idx:02}', [''.join(row) for row in grid],out))
    reference_file=folder/'references.json'
    refs=json.loads(reference_file.read_text()) if reference_file.exists() else {}
    for name,grid,out in tests:
        inp=str(len(grid))+'\n'+'\n'.join(grid)+'\n'; answer='\n'.join(out)+'\n'
        length=c.validate(grid,answer)
        ref={'length':length,'s':1.0,'source':'known feasible witness; not proven optimum'}
        if name=='sample': ref.update(length=18,source='statement proven shortest')
        refs[c.key(grid)]=ref
        (folder/f'{name}.in').write_text(inp)
        (folder/f'{name}.ref').write_text(json.dumps(ref)+'\n')
        (folder/f'{name}.sol').write_text(answer)
    (folder/'references.json').write_text(json.dumps(refs,indent=2)+'\n')
    print(f'wrote {len(tests)} solvable cases with fixed references')

if __name__=='__main__': generate_tests(Path(__file__).resolve().parent/'tests')
