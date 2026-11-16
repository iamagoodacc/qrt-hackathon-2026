#include <bits/stdc++.h>
using namespace std;

namespace CONFIG {
constexpr double WIDTH_FACTOR = 1.2;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    vector<int> side(n), order(n);
    int total = 0, largest = 0;
    for (int &s : side) {
        cin >> s;
        total += s * s;
        largest = max(largest, s);
    }
    iota(order.begin(), order.end(), 0);
    stable_sort(order.begin(), order.end(), [&](int a, int b) {
        return side[a] > side[b];
    });
    int width = max(largest, int(ceil(CONFIG::WIDTH_FACTOR * sqrt(total))));
    vector<pair<int, int>> answer(n);
    int row = 0, height = 0, col = 0;
    for (int i : order) {
        if (col + side[i] > width) {
            row += height;
            height = col = 0;
        }
        answer[i] = {row, col};
        col += side[i];
        height = max(height, side[i]);
    }
    for (auto [x, y] : answer) cout << x << ' ' << y << '\n';
}
