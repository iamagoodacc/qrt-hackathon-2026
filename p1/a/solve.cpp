#include <bits/stdc++.h>
using namespace std;

namespace P1_CONFIG {
constexpr int WIDTH_TRIALS = 128;
constexpr double TIME_BUDGET_S = 1.5;
}

vector<vector<int>> pack_squares(vector<int> squares) {
    const auto started = chrono::steady_clock::now();
    const int n = (int)squares.size();
    if (!n) return {};
    vector<int> order(n);
    iota(order.begin(), order.end(), 0);
    stable_sort(order.begin(), order.end(), [&](int a, int b) { return squares[a] > squares[b]; });
    long long sum = 0, largest = squares[order[0]];
    long double total = 0;
    for (int s : squares) { sum += s; total += (long double)s*s; }
    vector<vector<int>> best(n, vector<int>(2));
    long double bestArea = numeric_limits<long double>::infinity();
    // Descending next-fit shelves; widths cover a compact box through a single row.
    vector<long long> widths = {largest, sum};
    long long upper = min(sum, max(largest, (long long)(2*sqrt(total))));
    for (int k = 0; k <= P1_CONFIG::WIDTH_TRIALS; ++k)
        widths.push_back(largest + (upper-largest)*k/P1_CONFIG::WIDTH_TRIALS);
    for (long long width : widths) {
        if (isfinite(bestArea) && chrono::duration<double>(chrono::steady_clock::now()-started).count() > P1_CONFIG::TIME_BUDGET_S) break;
        vector<vector<int>> candidate(n, vector<int>(2));
        long long row = 0, col = 0, shelfHeight = 0, usedWidth = 0;
        bool representable = true;
        for (int id : order) {
            long long s = squares[id];
            if (col+s > width) { row += shelfHeight; col = 0; shelfHeight = 0; }
            if (row > INT_MAX || col > INT_MAX) { representable = false; break; }
            candidate[id] = {(int)row, (int)col};
            col += s;
            shelfHeight = max(shelfHeight, s);
            usedWidth = max(usedWidth, col);
        }
        long double area = (long double)(row+shelfHeight)*usedWidth;
        if (representable && area < bestArea) { bestArea = area; best = move(candidate); }
    }
    return best;
}

#ifdef LOCAL
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int n;
    if (!(cin >> n)) return 0;
    vector<int> squares(n);
    for (int &s : squares) cin >> s;
    for (auto &p : pack_squares(squares)) cout << p[0] << ' ' << p[1] << '\n';
}
#endif
