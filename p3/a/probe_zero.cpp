#include <bits/stdc++.h>
using namespace std;

// Unvalidated interface experiment. Output length and value constraints are unknown.
namespace P3_CONFIG {
constexpr float EXPERIMENT_VALUE = 0.0f;
}

vector<float> probe(float t) {
    (void)t;
    return {P3_CONFIG::EXPERIMENT_VALUE};
}

#ifdef LOCAL
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    float t;
    if (!(cin >> t)) return 0;
    for (float value : probe(t)) cout << value << '\n';
}
#endif
