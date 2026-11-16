// p0-b: square packing. Best-fit skyline decoder inside a W x H box + randomised search
// over box sizes (smallest area first). Build: g++ -std=c++17 -O2 -o solve solve.cpp
#include <bits/stdc++.h>
using namespace std;

// ---------------- CONFIG ----------------
struct Config {
    double time_budget_s = 1.6;   // ~80% of the 2 s limit
    uint64_t seed = 12345;
    double p_not_largest = 0.10;  // chance to pick a smaller fitting square than the largest
    double p_flip_side = 0.15;    // chance to place on the "wrong" side of the gap
    int tries_per_target = 64;    // random decodes per (W,H) before moving to the next target
    double max_aspect = 4.0;      // skip boxes with H/W or W/H above this
};
// ----------------------------------------

static mt19937_64 rng;
static inline double urand() { return (rng() >> 11) * (1.0 / 9007199254740992.0); }

int n, MS;                 // MS = max side
vector<int> side;
long long S;
int cnt0[32];

struct Seg { int x, w, y; };
struct Place { int s, x, y; };   // x = horizontal (output Y), y = vertical (output X)

// Decode into W x H (H may be INT_MAX/2 for strip). Returns true if all placed.
// Fills `out` with placements; `top` = max height used.
bool decode(int W, int H, const Config& cfg, bool randomize, vector<Place>& out, int& top) {
    int cnt[32];
    memcpy(cnt, cnt0, sizeof cnt);
    int left = n;
    long long budget = (H >= INT_MAX / 4) ? LLONG_MAX : (long long)W * H - S;
    long long waste = 0;
    static vector<Seg> sky;
    sky.clear();
    sky.push_back({0, W, 0});
    out.clear();
    top = 0;
    while (left > 0) {
        // lowest segment (leftmost on ties)
        int bi = 0;
        for (int i = 1; i < (int)sky.size(); i++)
            if (sky[i].y < sky[bi].y) bi = i;
        Seg g = sky[bi];
        int lim = min(g.w, H - g.y);
        int s = 0;
        for (int k = min(lim, MS); k >= 1; k--)
            if (cnt[k]) { s = k; break; }
        if (s > 0 && randomize && urand() < cfg.p_not_largest) {
            // pick a random smaller available size
            int k2 = s - 1;
            int cands[32], nc = 0;
            for (; k2 >= 1; k2--) if (cnt[k2]) cands[nc++] = k2;
            if (nc) s = cands[rng() % nc];
        }
        if (s == 0) {
            // raise to lower neighbour
            int ly = (bi > 0) ? sky[bi - 1].y : INT_MAX;
            int ry = (bi + 1 < (int)sky.size()) ? sky[bi + 1].y : INT_MAX;
            int ny = min(ly, ry);
            if (ny == INT_MAX || g.y >= H) return false;   // nothing fits anywhere
            if (ny > H) ny = H;
            waste += (long long)(ny - g.y) * g.w;
            if (waste > budget) return false;
            sky[bi].y = ny;
            // merge
            if (bi + 1 < (int)sky.size() && sky[bi + 1].y == ny) {
                sky[bi].w += sky[bi + 1].w;
                sky.erase(sky.begin() + bi + 1);
            }
            if (bi > 0 && sky[bi - 1].y == ny) {
                sky[bi - 1].w += sky[bi].w;
                sky.erase(sky.begin() + bi);
            }
            continue;
        }
        cnt[s]--;
        left--;
        // choose side: next to taller neighbour
        int ly = (bi > 0) ? sky[bi - 1].y : INT_MAX;
        int ry = (bi + 1 < (int)sky.size()) ? sky[bi + 1].y : INT_MAX;
        bool placeLeft = ly >= ry;
        if (randomize && urand() < cfg.p_flip_side) placeLeft = !placeLeft;
        int px = placeLeft ? g.x : g.x + g.w - s;
        out.push_back({s, px, g.y});
        top = max(top, g.y + s);
        if (s == g.w) {
            sky[bi].y += s;
        } else if (placeLeft) {
            sky[bi] = {g.x, s, g.y + s};
            sky.insert(sky.begin() + bi + 1, Seg{g.x + s, g.w - s, g.y});
        } else {
            sky[bi] = {g.x, g.w - s, g.y};
            sky.insert(sky.begin() + bi + 1, Seg{g.x + g.w - s, s, g.y + s});
            bi++;
        }
        // merge bi with neighbours
        if (bi + 1 < (int)sky.size() && sky[bi + 1].y == sky[bi].y) {
            sky[bi].w += sky[bi + 1].w;
            sky.erase(sky.begin() + bi + 1);
        }
        if (bi > 0 && sky[bi - 1].y == sky[bi].y) {
            sky[bi - 1].w += sky[bi].w;
            sky.erase(sky.begin() + bi);
        }
    }
    return true;
}

int main() {
    const Config cfg;
    rng.seed(cfg.seed);
    auto t0 = chrono::steady_clock::now();
    auto elapsed = [&] { return chrono::duration<double>(chrono::steady_clock::now() - t0).count(); };

    if (!(cin >> n)) return 0;
    side.resize(n);
    S = 0;
    MS = 0;
    memset(cnt0, 0, sizeof cnt0);
    for (auto& s : side) { cin >> s; S += (long long)s * s; MS = max(MS, s); cnt0[s]++; }

    // Initial: uncapped strip packing for every width
    const int BIG = INT_MAX / 2;
    long long bestA = LLONG_MAX;
    vector<Place> best, tmp;
    int bestW = 0, bestH = 0;
    int maxW = max<long long>(MS, S / MS + MS);
    for (int W = MS; W <= maxW; W++) {
        int top;
        decode(W, BIG, cfg, false, tmp, top);
        long long A = (long long)W * top;
        if (A < bestA) { bestA = A; best = tmp; bestW = W; bestH = top; }
    }
    cerr << "initial area " << bestA << " S=" << S << "\n";

    // Targets: (W,H), W,H >= MS, S <= W*H < bestA
    long long iters = 0;
    while (elapsed() < cfg.time_budget_s) {
        vector<pair<long long, pair<int,int>>> targets;
        for (int W = MS; W <= maxW; W++) {
            int H0 = max<long long>(MS, (S + W - 1) / W);
            for (int H = H0; (long long)W * H < bestA; H++) {
                if (max(W, H) > cfg.max_aspect * min(W, H)) continue;
                targets.push_back({(long long)W * H, {W, H}});
            }
        }
        if (targets.empty()) break;
        sort(targets.begin(), targets.end());
        bool improved = false;
        for (auto& t : targets) {
            int W = t.second.first, H = t.second.second;
            for (int k = 0; k < cfg.tries_per_target; k++) {
                int top;
                iters++;
                if (decode(W, H, cfg, true, tmp, top)) {
                    bestA = t.first; best = tmp; bestW = W; bestH = H;
                    improved = true;
                    break;
                }
            }
            if (improved || elapsed() > cfg.time_budget_s) break;
        }
        (void)improved;
    }
    cerr << "final area " << bestA << " (" << bestW << "x" << bestH << ") iters=" << iters << "\n";

    // Assign input indices
    vector<vector<int>> bySize(32);
    for (int i = n - 1; i >= 0; i--) bySize[side[i]].push_back(i);
    vector<pair<int,int>> ans(n);
    for (auto& p : best) {
        int i = bySize[p.s].back(); bySize[p.s].pop_back();
        ans[i] = {p.y, p.x};
    }
    string o;
    for (auto& a : ans) o += to_string(a.first) + " " + to_string(a.second) + "\n";
    fputs(o.c_str(), stdout);
    return 0;
}
