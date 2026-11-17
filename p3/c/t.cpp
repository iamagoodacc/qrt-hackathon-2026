#include <bits/stdc++.h>
using namespace std;

vector<float> probe(float t) {
    static const vector<float> ts = {1.5f, 2.5f};
    static const vector<vector<float>> vs = {
        vector<float>{0.0f, 1.0f},
        vector<float>{3.0f, 4.0f}};
    size_t best = 0;
    for (size_t i = 1; i < ts.size(); i++) if (fabs(ts[i] - t) < fabs(ts[best] - t)) best = i;
    return vs[best];
}
