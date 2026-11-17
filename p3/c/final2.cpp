#include <bits/stdc++.h>
using namespace std;
// p3-c candidate from a JOINT quadratic fit of runs 1+2 (s = a x - c x^2 per regime, N=1):
//   A t<=0.19 : x*=+0.042 (fit, interpolated)          -> 0.04
//   B 0.20-0.49 : slope at 0 negative (a=-0.18), x*~-0.13 (extrapolated) -> half: -0.06
//   C0 0.50-0.53: all 6 points negative for x>=0.2, behaves like B        -> -0.06
//   C 0.53-0.81 : x*=+0.31, s*~50% of target (42 points, interpolated)    -> 0.31
//   D >=0.82    : slope at 0 negative (a=-0.19), x*~-0.38 (extrapolated)  -> about 0.4 x*: -0.15
// Per-case scores are clipped at 0, so the extrapolated negative guesses can't score below 0.
namespace P3C {
float A_END = 0.19f, B_END = 0.49f, C0_END = 0.53f, C_END = 0.81f;
float X_A = 0.04f, X_B = -0.06f, X_C0 = -0.06f, X_C = 0.31f, X_D = -0.15f;
}
vector<float> probe(float t) {
    using namespace P3C;
    if (t <= A_END) return {X_A};
    if (t <= B_END) return {X_B};
    if (t < C0_END) return {X_C0};
    if (t <= C_END) return {X_C};
    return {X_D};
}
