#include <bits/stdc++.h>
using namespace std;

// p3-b round 1: bet size x = xhat(t) * m, m cycles 0.5 / 1.0 / 1.6 by case index k = round(t/0.02).
vector<float> probe(float t) {
    int k = (int)lround(t / 0.02f);                 // case index 0..50 (case number = k+1)
    double xhat;
    if (t <= 0.19) xhat = 0.06;                      // regime A
    else if (t <= 0.49) xhat = 0.37 * t;             // regime B
    else if (t <= 0.81) xhat = 0.45 * t;             // regime C
    else xhat = 0.20;                                // regime D
    static const double M[3] = {0.5, 1.0, 1.6};
    return {(float)(xhat * M[k % 3])};
}

#ifdef LOCAL
int main() { float t; cin >> t; for (float x : probe(t)) cout << x << "\n"; }
#endif
