#include <bits/stdc++.h>
using namespace std;

// p3-b round 2: per regime, three levels cycling by case index k = round(t/0.02).
vector<float> probe(float t) {
    int k = (int)lround(t / 0.02f) % 3;
    static const double A[3] = {-0.03, 0.03, 0.06};    // t <= 0.18
    static const double B[3] = {-0.06, -0.15, -0.35};  // 0.20 <= t <= 0.52  (negative side)
    static const double C[3] = {0.22, 0.31, 0.40};     // 0.54 <= t <= 0.80
    static const double D[3] = {-0.05, -0.15, -0.35};  // t >= 0.82          (negative side)
    double x;
    if (t <= 0.19) x = A[k];
    else if (t <= 0.53) x = B[k];
    else if (t <= 0.81) x = C[k];
    else x = D[k];
    return {(float)x};
}

#ifdef LOCAL
int main() { float t; cin >> t; for (float x : probe(t)) cout << x << "\n"; }
#endif
