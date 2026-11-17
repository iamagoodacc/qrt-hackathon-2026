# p3-c notes (second agent; p3-b owns p3/LOG.md and the experiment sequence)

## What we know
- `vector<float> probe(float t)`; HackerRank main prints each float with DEFAULT ostream precision
  (6 significant digits): 0.123456789 -> 0.123457. Values needing more precision are rounded!
- Feedback per submission is NOISY; the official score = NOISELESS payoff of the LAST submission.
- normalized noisy score = raw_noisy / raw_noiseless_target, clipped to [0,1].

## Ideas to get more out of each submission (for p3-b / orchestrator)
1. MULTIPLEX: a submission runs every hidden test, each with its own t. If feedback is per test, make
   the output depend on t so each test evaluates a DIFFERENT candidate (e.g. candidate index = rank of t
   among the t values seen in "Run code", or hash of t). One submission = K measurements.
   Caveat: payoff may depend on t itself (different target per test), then compare only same-t rows.
2. If t looks like a time / step (0,1,2,...), the box may be a time series: we predict y(t);
   echo/affine probes (a*t+b) quickly show dependence on t.
3. Length probe: return k zeros for k = 1, 2, 5, 10, 100 in one submission via multiplexing
   (if the grader accepts any length) to find the expected dimension.
4. Noise: estimate sigma by repeating ONE fixed candidate in several tests/submissions; only trust
   differences > 2 sigma/sqrt(repeats). Final submission = best mean, not best single value.
5. Optimisation once the dimension d is known: coordinate search with step halving, or fit a
   quadratic along each coordinate from 3 points (x-h, x, x+h): x* = x - h(f+ - f-)/(2(f+ - 2f0 + f-)).

## Tools
- p3/c/mk.py: writes a paste-ready probe file from a spec, e.g.
    python3 p3/c/mk.py const 0.5,1,2 > p3/c/p.cpp      -> always returns {0.5,1,2}
    python3 p3/c/mk.py affine 2 1 > p3/c/p.cpp         -> returns {2*t+1}
    python3 p3/c/mk.py table "1.5:0,1;2.5:3,4" > ...   -> per-t lookup (nearest t), multiplexing
    python3 p3/c/mk.py zeros 10 > ...                  -> 10 zeros
  Every output is checked to compile with ./hrbuild.sh before use.
- p3/c/quad.py: 1-D quadratic step from three (x, payoff) points (no numpy needed).

## Log (20:25)
- p3/c/safe.cpp: insurance (x=t for t<=0.08, 1000x0.05 for t>=0.82, else 0). Built + checked with hrbuild.
- p3/parse.py: pasted "t=..., N=..., raw_noisy_score=..., raw_noiseless_target=..." lines -> rows of the
  existing p3/data.csv format (round,case,t,N,x,raw_noisy,target); --probe echo|scale:k|const:v|file:X.cpp[*k].
- p3/c/fit.py: per-t quadratic model; p3/c/xstar.cpp = conservative x*(t) table (f=1/8); xstar_half.cpp = 0.5x*.
- p3/IDEAS.md: probes for p3-b (run xstar, xstar_half, N/shape tests).
