#include <bits/stdc++.h>
using namespace std;
// p3 FINAL from runs 1+2 (p3/blackbox_analysis.md + run2). Per-case scores are clipped at 0, so an
// untested guess can't score below returning 0.
namespace P3CFG {
float A_END = 0.19f, B_END = 0.49f, C0_END = 0.53f, C_END = 0.81f;  // regime boundaries
float X_A = 0.04f;        // t<=0.19: small positive ~ noise-level positive in both runs
float X_B = -0.03f;       // 0.20-0.48: every positive x lost, more x = worse -> try small negative
float K_C0 = 0.225f;      // 0.50-0.52: x=0.45t and 0.72t lost there -> use half (0.225t)
float K_C = 0.45f;        // 0.53-0.80: x=0.45t (x0.5..x1.6 all positive, mean +0.037)
float X_D = 0.05f; int N_D = 1000;  // >=0.82: single x>=0.1 lost; 0.05 x1000 scored +0.0079 at t=1
}
vector<float> probe(float t) {
    using namespace P3CFG;
    if (t <= A_END) return {X_A};
    if (t <= B_END) return {X_B};
    if (t < C0_END) return {K_C0 * t};
    if (t <= C_END) return {K_C * t};
    return vector<float>(N_D, X_D);
}
