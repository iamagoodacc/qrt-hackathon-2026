# P3: Black box

(Transcribed from the HackerRank screenshot `p3/statement_1.png`. This is the WHOLE statement shown.)

> Query the black box to maximise payoff. Beware the noise.
>
> *Bonus eligibility uses your official final noiseless score for the whole question. Individual
> noisy feedback values do not determine eligibility. The normalized noisy score is
> raw_noisy_score / raw_noiseless_target, clipped to the range 0-1*

## HackerRank C++ stub (language C++23)
```cpp
/*
 * Complete the 'probe' function below.
 *
 * The function is expected to return a FLOAT_ARRAY.
 * The function accepts FLOAT t as parameter.
 */
vector<float> probe(float t) {

}
```
HackerRank's hidden main() is copied exactly into `p3/hr_main.cpp`. Build with `./hrbuild.sh`.
- Input: ONE line with a float `t`. Output: the returned floats, one per line.
- There is NO black-box function in the hidden code. The "black box" is HackerRank's grader:
  the only way to query it is to SUBMIT and read the feedback.

## What we know / don't know
- We return a list of floats given one float `t`. A hidden function turns that into a "payoff".
- Feedback after submitting is NOISY. The official score (and the bonus gate) uses the
  NOISELESS payoff of the FINAL submission -> the LAST submission must be our best candidate.
- No local checker is possible until we know more: the scoring is hidden on HackerRank.
- UNKNOWN: what `t` means, how many floats to return, what the payoff function looks like.

## MISSING (orchestrator to add)
- [ ] What "Run code" shows with the empty stub (sample input t, output, any feedback message).
- [ ] Time limit, and whether the number of submissions is limited.

## How we work on this one (orchestrator notes)
- The feedback loop is HackerRank itself, so a human must submit each experiment.
- One Claude agent (`p3-b`) designs experiments: it keeps `p3/LOG.md` (one row per submission:
  what was returned, the feedback values), proposes the next probe, and writes paste-ready code.
- The orchestrator pastes the code, submits, and pastes the feedback back to the agent.
- Noise: repeat promising candidates to average the noise before trusting a difference.
