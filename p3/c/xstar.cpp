#include <bits/stdc++.h>
using namespace std;

// p3-c candidate x*(t): constant (N=1) at the CONSERVATIVE fitted optimum.
// Model per t: payoff s(x) = Teff*(2z - z^2), z = x/u(t), Teff = 0.125*target (best constant earns ~1/8),
// u(t) solved from the echo run (x = t overshoots for t >= 0.1), then a +-2-case rolling median.
// t <= 0.08 keeps the measured-positive echo x = t. Table index k = round(t / 0.02), k = 0..50.
namespace P3XS {
const float STEP = 0.02f;
const vector<float> X = {0.0000f, 0.0200f, 0.0400f, 0.0600f, 0.0800f, 0.0478f, 0.0462f, 0.0452f, 0.0452f, 0.0452f, 0.0492f, 0.0515f, 0.0562f, 0.0562f, 0.0630f, 0.0659f, 0.0699f, 0.0788f, 0.0788f, 0.0807f, 0.0820f, 0.0858f, 0.0858f, 0.0909f, 0.0909f, 0.1332f, 0.1553f, 0.1575f, 0.1778f, 0.1778f, 0.1835f, 0.1835f, 0.1964f, 0.1992f, 0.2291f, 0.2291f, 0.2291f, 0.2291f, 0.2291f, 0.2230f, 0.1298f, 0.0874f, 0.0788f, 0.0788f, 0.0788f, 0.0657f, 0.0657f, 0.0657f, 0.0984f, 0.1000f, 0.1016f};
}

vector<float> probe(float t) {
    int k = (int)lround(t / P3XS::STEP);
    k = max(0, min((int)P3XS::X.size() - 1, k));
    return vector<float>{P3XS::X[k]};
}
