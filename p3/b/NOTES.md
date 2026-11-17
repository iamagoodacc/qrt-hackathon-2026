# p3-b notes
- Task: `vector<float> probe(float t)`; hidden grader turns our floats into a payoff; feedback is noisy.
  Final score = NOISELESS payoff of the LAST submission -> last submission must be the best candidate.
- No local checker. Experiments go through the orchestrator; log in p3/LOG.md.
- Plan: (1) Run code with echo to learn t and grader messages; (2) learn the expected output length;
  (3) map payoff vs. returned values with designed probes (vary one thing at a time, repeat to average noise).
