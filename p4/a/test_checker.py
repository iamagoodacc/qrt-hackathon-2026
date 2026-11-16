import importlib.util
import math
from pathlib import Path
spec=importlib.util.spec_from_file_location('checker',Path(__file__).resolve().parents[1]/'checker.py')
c=importlib.util.module_from_spec(spec); spec.loader.exec_module(c)
grid=['......','.W....','....B.','......','.B....','....W.']
answer='......\nF-7...\nL7L-7.\n.|..|.\n.L-7L7\n...L-J\n'
assert c.validate(grid,answer)==18

def bad(g,out):
    try: c.validate(g,out)
    except c.Invalid: return
    raise AssertionError((g,out))
bad(grid,'......\n'*6)
bad(grid,answer.replace('F','X'))
bad(grid,answer.replace('F','L'))
bad(grid,answer+'\n')
bad(grid,answer.replace('|','.'))
g=[list(row) for row in grid];g[1][0]='W';bad([''.join(row) for row in g],answer)
g=[list(row) for row in grid];g[1][1]='B';bad([''.join(row) for row in g],answer)
# Two disconnected 2x2 cycles; each has degree two but the global rule rejects it.
bad(['......']*6,'F7.F7.\nLJ.LJ.\n......\n......\n......\n......\n')
# Adjacent cells need not be connected: the supplied sample contains such cells.
assert c.score('6\n'+'\n'.join(grid)+'\n',answer,1)==1
assert math.isclose(c.score('6\n'+'\n'.join(grid)+'\n',answer,6),.5+.5*math.exp(-1))
for test in (Path(__file__).resolve().parents[1]/'tests').glob('*.in'):
    assert c.score(test.read_text(),test.with_suffix('.sol').read_text())==1
print('checker: sample, malformed/broken/multiple loops, both pearl rules, runtime factor, all generated witnesses passed')
