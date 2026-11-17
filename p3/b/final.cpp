#include <bits/stdc++.h>
using namespace std;

// p3-b FINAL candidate (after rounds 0-1): one bet size per regime of t.
// CONFIG: bet size per regime (update after each probe round).
namespace P3CFG {
const double X_A = 0.03;    // t <= 0.18 : slope slightly positive, fitted x* ~ +0.04
const double X_B = -0.06;   // 0.20 <= t <= 0.52 : every positive x lost -> small negative bet
const double X_C = 0.31;    // 0.54 <= t <= 0.80 : fitted x* ~ 0.31 (two fits agree)
const double X_D = -0.05;   // t >= 0.82 : every positive x lost -> small negative bet
}

vector<float> probe(float t) {
    double x;
    if (t <= 0.19) x = P3CFG::X_A;
    else if (t <= 0.53) x = P3CFG::X_B;
    else if (t <= 0.81) x = P3CFG::X_C;
    else x = P3CFG::X_D;
    return {(float)x};
}

#ifdef LOCAL
int main() { float t; cin >> t; for (float x : probe(t)) cout << x << "\n"; }
#endif
