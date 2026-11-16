import importlib.util
import math
from pathlib import Path
spec = importlib.util.spec_from_file_location('checker', Path(__file__).resolve().parents[1]/'checker.py')
c = importlib.util.module_from_spec(spec)
spec.loader.exec_module(c)
def inp(req, forbidden=()):
    return '\n'.join([str(len(req)), *req, str(len(forbidden)), *forbidden])+'\n'
def invalid(req, forbidden, out):
    try:
        c.score(inp(req, forbidden), out)
    except c.Invalid:
        return
    raise AssertionError((req, forbidden, out))
assert math.isclose(c.score(inp(['ACE','FIG','OAK']), 'A C E\nF I G\nO A K\n'), 9/(1.7*9))
grid = ['QATSUR','YFLRUB','ILJAVA','UREUCW','GELLSS','YPNOHT']
words = ['PYTHON','JAVA','RUBY','PERL','RUST','SWIFT','SCALA','LUA']
assert math.isclose(c.score(inp(words), '\n'.join(grid)), 35/(1.7*36))
for dr, dc in c.DIRECTIONS:
    g = ['ABC','DEF','GHI']
    w = ''.join(g[(2+k*dr)%3][(2+k*dc)%3] for k in range(3))
    assert c.contains(g, w)
    invalid(['ABC'], [w], '\n'.join(g))
invalid(['AAAA'], [], 'A A\nA A\n')
invalid(['AB'], ['ABABAB'], 'A B\nC D\n')
invalid(['ZZ'], [], 'AB\nCD\n')
for out in ['', 'a\n', 'A B\n', 'A 1\nB C\n', 'A B\nC D\n\n', 'A B C\nD E F\n']:
    invalid(['A'], [], out)
assert c.score(inp(['A','A']), 'A\n') == 1
print('checker: samples, all eight wrapping directions, multiwrap forbidden, minimum N, malformed grids passed')
