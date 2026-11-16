#include <bits/stdc++.h>
using namespace std;

namespace P2_CONFIG {
constexpr double TIME_BUDGET_S = 1.5;
constexpr int RANDOM_ATTEMPTS = 64;
constexpr unsigned SEED = 2201;
constexpr int ROW_SPACING = 2;
}

vector<vector<char>> generate_grid(vector<string> required_words, vector<string> forbidden_words) {
    const auto started = chrono::steady_clock::now();
    mt19937 rng(P2_CONFIG::SEED);
    int longest = 1;
    for (const auto &w : required_words) longest = max(longest, (int)w.size());
    int n = max(longest, P2_CONFIG::ROW_SPACING*(int)required_words.size());
    auto hasForbidden = [&](const vector<vector<char>> &grid) {
        for (const string &w : forbidden_words)
            for (int r = 0; r < n; ++r) for (int c = 0; c < n; ++c) {
                if (grid[r][c] != w[0]) continue;
                for (int dr = -1; dr <= 1; ++dr) for (int dc = -1; dc <= 1; ++dc) {
                    if (!dr && !dc) continue;
                    bool match = true;
                    int rr = r, cc = c;
                    for (char ch : w) {
                        if (grid[rr][cc] != ch) { match = false; break; }
                        rr = (rr+dr+n)%n;
                        cc = (cc+dc+n)%n;
                    }
                    if (match) return true;
                }
            }
        return false;
    };
    vector<vector<char>> grid;
    // Simple safe construction: one word per separated row, try each constant filler.
    for (int trial = 0; trial < 26+P2_CONFIG::RANDOM_ATTEMPTS; ++trial) {
        char filler = 'Z'-trial%26;
        grid.assign(n, vector<char>(n, filler));
        if (trial >= 26)
            for (auto &row : grid) for (char &ch : row) ch = 'A'+rng()%26;
        vector<int> rows(required_words.size());
        iota(rows.begin(), rows.end(), 0);
        if (trial >= 26) shuffle(rows.begin(), rows.end(), rng);
        for (int i = 0; i < (int)required_words.size(); ++i) {
            int col = trial < 26 ? 0 : rng()%n;
            for (int k = 0; k < (int)required_words[i].size(); ++k)
                grid[P2_CONFIG::ROW_SPACING*rows[i]][(col+k)%n] = required_words[i][k];
        }
        if (!hasForbidden(grid)) return grid;
        if (chrono::duration<double>(chrono::steady_clock::now()-started).count() > P2_CONFIG::TIME_BUDGET_S) break;
    }
    return grid;
}

#ifdef LOCAL
int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    int r, f;
    if (!(cin >> r)) return 0;
    vector<string> required(r);
    for (auto &w : required) cin >> w;
    cin >> f;
    vector<string> forbidden(f);
    for (auto &w : forbidden) cin >> w;
    for (const auto &row : generate_grid(required, forbidden)) {
        for (int j = 0; j < (int)row.size(); ++j) {
            if (j) cout << ' ';
            cout << row[j];
        }
        cout << '\n';
    }
}
#endif
