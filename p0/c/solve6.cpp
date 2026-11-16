// p0-c v2: square packing by fixed-container skyline DFS (not SA).
// For a container W x H, fill the lowest-leftmost skyline gap with a remaining square
// (distinct sizes, ordered by a fit heuristic + random noise) or waste the gap up to its
// lower neighbour. Prune when wasted area exceeds W*H - S. Randomised restarts with node
// limits; shrink the target area while time remains.
#include <bits/stdc++.h>
using namespace std;

namespace CONFIG {
double TIME_BUDGET_S = 1.6;          // stop searching after this many seconds (argv[1] overrides)
double START_NODES = 2000;           // node limit per DFS attempt in the first round
double NODE_GROWTH = 2.0;            // node limit multiplier after a round without improvement
double MAX_NODES = 50000;            // node limit cap; beyond it we just keep restarting
double BONUS_EXACT = 100;            // key bonus: square exactly fills the gap width
double BONUS_LEVEL = 50;             // key bonus: square top is level with the left neighbour
double NOISE = 4.0;                  // random key noise (in side units) on restarts
int TALLER_SIDE = 1;                 // 1 = place the square against the taller neighbour, 0 = always left
constexpr int MAXS = 30;             // max side from the statement
}

static chrono::steady_clock::time_point T0;
static double elapsed() {
    return chrono::duration<double>(chrono::steady_clock::now() - T0).count();
}
static uint64_t rngState = 88172645463325252ULL;
static inline double urand() {
    rngState ^= rngState << 7; rngState ^= rngState >> 9;
    return (rngState >> 11) * (1.0 / 9007199254740992.0);
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
double noise;
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
    int maxS = min(min(g, H - h0), CONFIG::MAXS);
    bool right = CONFIG::TALLER_SIDE && rightH > leftH;
    int sideH = right ? rightH : leftH;
    // candidates: size s > 0, or 0 = waste the gap
    pair<double, int> cand[CONFIG::MAXS + 1];
    int k = 0;
    for (int s = maxS; s >= 1; s--) {
        if (!cnt[s]) continue;
        double key = s;
        if (s == g) key += CONFIG::BONUS_EXACT;
        if (h0 + s == sideH) key += CONFIG::BONUS_LEVEL;
        if (noise > 0) key += noise * urand();
        cand[k++] = {key, s};
    }
    sort(cand, cand + k, [](auto &a, auto &b) { return a.first > b.first; });
    for (int c = 0; c < k; c++) {
        int s = cand[c].second;
        cnt[s]--; remaining--;
        int px = right ? e + 1 - s : x;
        for (int i = px; i < px + s; i++) sky[i] += s;
        placed.push_back({s, h0, px});
        if (dfs()) return true;
        placed.pop_back();
        for (int i = px; i < px + s; i++) sky[i] -= s;
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
bool tryPack(int w, int h, long long limit, double nz, vector<pair<int, int>> &out, long long &area) {
    W = w; H = h;
    budget = (long long)W * H - S;
    if (budget < 0) return false;
    wasted = 0;
    sky.assign(W, 0);
    memcpy(cnt, cntInit, sizeof cnt);
    remaining = n;
    nodes = 0; nodeLimit = limit; aborted = false; noise = nz;
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
    // experiments: ./solve NAME=value ... (no args on HackerRank -> CONFIG defaults)
    for (int a = 1; a < argc; a++) {
        string kv = argv[a]; size_t eq = kv.find('='); if (eq == string::npos) continue;
        string k = kv.substr(0, eq); double v = atof(kv.c_str() + eq + 1);
        if (k == "TIME") CONFIG::TIME_BUDGET_S = v;
        else if (k == "START") CONFIG::START_NODES = v;
        else if (k == "GROWTH") CONFIG::NODE_GROWTH = v;
        else if (k == "MAXN") CONFIG::MAX_NODES = v;
        else if (k == "EXACT") CONFIG::BONUS_EXACT = v;
        else if (k == "LEVEL") CONFIG::BONUS_LEVEL = v;
        else if (k == "NOISE") CONFIG::NOISE = v;
        else if (k == "TALLER") CONFIG::TALLER_SIDE = (int)v;
        else if (k == "SEED") rngState ^= (uint64_t)v * 0x9E3779B97F4A7C15ULL;
    }
    if (!(cin >> n)) return 0;
    side.resize(n);
    m = 0; S = 0;
    for (int &s : side) { cin >> s; m = max(m, s); S += (long long)s * s; cntInit[s]++; }

    vector<pair<int, int>> best, cur;
    long long bestArea = LLONG_MAX, area;

    // Phase 1: greedy (unbounded height) for every width.
    int maxW = (int)min<long long>(S / m + m, 100000);
    for (int w = m; w <= maxW; w++) {
        if (tryPack(w, INT_MAX / 4, LLONG_MAX, 0, cur, area) && area < bestArea) {
            bestArea = area; best = cur;
        }
        if (elapsed() > CONFIG::TIME_BUDGET_S * 0.25) break;
    }

    // Phase 2: containers with area below the best, most slack first.
    double limit = CONFIG::START_NODES;
    int round = 0;
    long long totalNodes = 0;
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
            bool ok = tryPack(w, (int)(a / w), (long long)limit, round == 0 ? 0 : CONFIG::NOISE, cur, area);
            totalNodes += nodes;
            if (ok && area < bestArea) {
                bestArea = area; best = cur; improved = true;
                break;
            }
        }
        if (!improved) { limit = min(limit * CONFIG::NODE_GROWTH, CONFIG::MAX_NODES); round++; }
    }
    { int R = 0, C = 0; for (int i = 0; i < n; i++) { R = max(R, best[i].first + side[i]); C = max(C, best[i].second + side[i]); } cerr << R << "x" << C << " "; }
    cerr << "area " << bestArea << " S " << S << " rounds " << round << " nodes " << totalNodes << " t " << elapsed() << "\n";
    for (auto &[r, c] : best) cout << r << ' ' << c << '\n';
}
