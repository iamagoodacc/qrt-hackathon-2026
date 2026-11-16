// p0-c: square packing by fixed-container skyline DFS (not SA).
// For a container W x H, fill the lowest-leftmost skyline gap with a remaining square
// (distinct sizes, biggest first) or waste the gap up to its lower neighbour.
// Prune when wasted area exceeds W*H - S. Shrink the target area while time remains.
#include <bits/stdc++.h>
using namespace std;

namespace CONFIG {
double TIME_BUDGET_S = 1.6;               // stop searching after this many seconds (argv[1] overrides, for experiments)
constexpr long long START_NODES = 2000;   // node limit per container in the first round
constexpr double NODE_GROWTH = 2.0;       // node limit multiplier per round
constexpr int MAXS = 30;                  // max side from the statement
}

static chrono::steady_clock::time_point T0;
static double elapsed() {
    return chrono::duration<double>(chrono::steady_clock::now() - T0).count();
}

int n, m;
long long S;
vector<int> side;
int cntInit[CONFIG::MAXS + 1];

// ---- search state ----
int W, H;
long long budget, wasted;
vector<int> sky;                 // height per column
int cnt[CONFIG::MAXS + 1];
int remaining;
long long nodes, nodeLimit;
bool aborted;
vector<array<int, 3>> placed;    // (size, row, col) stack

bool dfs() {
    if (remaining == 0) return true;
    if (++nodes > nodeLimit) { aborted = true; return false; }
    // lowest-leftmost gap
    int x = 0;
    for (int i = 1; i < W; i++) if (sky[i] < sky[x]) x = i;
    int h0 = sky[x];
    int e = x;
    while (e + 1 < W && sky[e + 1] == h0) e++;
    int g = e - x + 1;
    int leftH = x > 0 ? sky[x - 1] : H;
    int rightH = e + 1 < W ? sky[e + 1] : H;
    int room = H - h0;
    int maxS = min(g, room);
    for (int s = min(maxS, CONFIG::MAXS); s >= 1; s--) {
        if (!cnt[s]) continue;
        cnt[s]--; remaining--;
        for (int i = x; i < x + s; i++) sky[i] += s;
        placed.push_back({s, h0, x});
        if (dfs()) return true;
        placed.pop_back();
        for (int i = x; i < x + s; i++) sky[i] -= s;
        cnt[s]++; remaining++;
        if (aborted) return false;
    }
    // waste the gap up to its lower neighbour
    int nh = min(leftH, rightH);
    long long w = (long long)g * (nh - h0);
    if (w <= 0 || wasted + w > budget) return false;
    wasted += w;
    for (int i = x; i <= e; i++) sky[i] = nh;
    bool ok = dfs();
    if (ok) return true;
    for (int i = x; i <= e; i++) sky[i] = h0;
    wasted -= w;
    return false;
}

// Try to pack everything into W x H. On success fills `out` (row, col per input index).
bool tryPack(int w, int h, long long limit, vector<pair<int, int>> &out, long long &area) {
    W = w; H = h;
    budget = (long long)W * H - S;
    if (budget < 0) return false;
    wasted = 0;
    sky.assign(W, 0);
    memcpy(cnt, cntInit, sizeof cnt);
    remaining = n;
    nodes = 0; nodeLimit = limit; aborted = false;
    placed.clear();
    if (!dfs()) return false;
    // map placements back to input indices
    vector<vector<int>> bySize(CONFIG::MAXS + 1);
    for (int i = 0; i < n; i++) bySize[side[i]].push_back(i);
    out.assign(n, {0, 0});
    int maxRow = 0, maxCol = 0;
    for (auto &p : placed) {
        int i = bySize[p[0]].back(); bySize[p[0]].pop_back();
        out[i] = {p[1], p[2]};
        maxRow = max(maxRow, p[1] + p[0]);
        maxCol = max(maxCol, p[2] + p[0]);
    }
    area = (long long)maxRow * maxCol;
    return true;
}

int main(int argc, char **argv) {
    T0 = chrono::steady_clock::now();
    if (argc > 1) CONFIG::TIME_BUDGET_S = atof(argv[1]);
    if (!(cin >> n)) return 0;
    side.resize(n);
    m = 0; S = 0;
    for (int &s : side) { cin >> s; m = max(m, s); S += (long long)s * s; cntInit[s]++; }

    vector<pair<int, int>> best, cur;
    long long bestArea = LLONG_MAX, area;

    // Phase 1: greedy (unbounded height) for every width.
    int maxW = (int)min<long long>(S / m + m, 100000);
    for (int w = m; w <= maxW; w++) {
        if (tryPack(w, INT_MAX / 4, LLONG_MAX, cur, area) && area < bestArea) {
            bestArea = area; best = cur;
        }
        if (elapsed() > CONFIG::TIME_BUDGET_S * 0.25) break;
    }

    // Phase 2: containers with area below the best, most slack first, growing node limits.
    double limit = CONFIG::START_NODES;
    while (elapsed() < CONFIG::TIME_BUDGET_S) {
        long long target = bestArea - 1;
        vector<pair<long long, int>> cands;  // (container area, width)
        for (int w = m; (long long)w * m <= target; w++) {
            long long h = target / w;
            if (h < m || (long long)w * h < S) continue;
            cands.push_back({(long long)w * h, w});
        }
        if (cands.empty()) break;
        sort(cands.rbegin(), cands.rend());
        bool improved = false;
        for (auto &[a, w] : cands) {
            if (elapsed() > CONFIG::TIME_BUDGET_S) break;
            if (a >= bestArea) continue;
            if (tryPack(w, (int)(a / w), (long long)limit, cur, area) && area < bestArea) {
                bestArea = area; best = cur; improved = true;
                break;
            }
        }
        if (!improved) limit *= CONFIG::NODE_GROWTH;
    }

    cerr << "area " << bestArea << " S " << S << " lastLimit " << limit << " t " << elapsed() << "\n";
    for (auto &[r, c] : best) cout << r << ' ' << c << '\n';
}
