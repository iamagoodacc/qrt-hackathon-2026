// p0-b: square packing. Skyline packing inside a W x H box.
// Phase 1: random best-fit decodes over box sizes (smallest area first) -> quick good answer.
// Phase 2: depth-first search over skyline decisions (which size at the lowest gap, or waste it)
//          with waste budget W*H - S and a lower-bound prune; node limit doubles each pass.
// Build: g++ -std=c++17 -O2 -o solve solve.cpp
#include <bits/stdc++.h>
using namespace std;

// ---------------- CONFIG ----------------
struct Config {
    double time_budget_s = 1.6;   // ~80% of the 2 s limit
    uint64_t seed = 12345;
    double p_not_largest = 0.10;  // phase 1: chance to pick a smaller fitting square
    double p_flip_side = 0.15;    // phase 1: chance to place on the "wrong" side of the gap
    int tries_per_target = 64;    // phase 1: random decodes per (W,H)
    double phase1_frac = 0.15;    // share of time spent in phase 1
    double max_aspect = 6.0;      // skip boxes with H/W or W/H above this
    long long dfs_nodes0 = 2000;  // phase 2: node limit per target on the first pass (x2 per pass)
    double p_shuffle = 0.05;      // phase 2: chance a node tries its children in rotated order
    double limit_growth = 2.0;    // phase 2: node limit multiplier after a pass with no success
};
// ----------------------------------------

static mt19937_64 rng;
static inline double urand() { return (rng() >> 11) * (1.0 / 9007199254740992.0); }

int n, MS;
vector<int> side;
long long S;
int cnt0[32];

struct Seg { int x, w, y; };
struct Place { int s, x, y; };   // x = horizontal (output Y), y = vertical (output X)

static inline void mergeAt(vector<Seg>& sky, int bi) {
    if (bi + 1 < (int)sky.size() && sky[bi + 1].y == sky[bi].y) {
        sky[bi].w += sky[bi + 1].w;
        sky.erase(sky.begin() + bi + 1);
    }
    if (bi > 0 && sky[bi - 1].y == sky[bi].y) {
        sky[bi - 1].w += sky[bi].w;
        sky.erase(sky.begin() + bi);
    }
}

// Phase-1 decoder: random choices, abort when waste exceeds budget.
bool decodeRand(int W, int H, const Config& cfg, bool randomize, vector<Place>& out, int& top) {
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
        int bi = 0;
        for (int i = 1; i < (int)sky.size(); i++)
            if (sky[i].y < sky[bi].y) bi = i;
        Seg g = sky[bi];
        int lim = min(g.w, H - g.y);
        int s = 0;
        for (int k = min(lim, MS); k >= 1; k--)
            if (cnt[k]) { s = k; break; }
        if (s > 0 && randomize && urand() < cfg.p_not_largest) {
            int cands[32], nc = 0;
            for (int k2 = s - 1; k2 >= 1; k2--) if (cnt[k2]) cands[nc++] = k2;
            if (nc) s = cands[rng() % nc];
        }
        if (s == 0) {
            int ly = (bi > 0) ? sky[bi - 1].y : INT_MAX;
            int ry = (bi + 1 < (int)sky.size()) ? sky[bi + 1].y : INT_MAX;
            int ny = min(ly, ry);
            if (ny == INT_MAX || g.y >= H) return false;
            if (ny > H) ny = H;
            waste += (long long)(ny - g.y) * g.w;
            if (waste > budget) return false;
            sky[bi].y = ny;
            mergeAt(sky, bi);
            continue;
        }
        cnt[s]--;
        left--;
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
        mergeAt(sky, bi);
    }
    return true;
}

// ---------------- Phase 2: DFS ----------------
const int MAXSEG = 256;
const int MAXDEPTH = 4096;
struct Sky { Seg seg[MAXSEG]; int ns; };
static Sky stk[MAXDEPTH];
static int dW, dH;
static long long dBudget, dWaste, dNodes, dLimit;
static int dCnt[32], dLeft;
static bool dAbort;
static vector<Place> dPath;
static Config gcfg;
static chrono::steady_clock::time_point gT0;

static inline void skyMerge(Sky& k, int bi) {
    if (bi + 1 < k.ns && k.seg[bi + 1].y == k.seg[bi].y) {
        k.seg[bi].w += k.seg[bi + 1].w;
        memmove(&k.seg[bi + 1], &k.seg[bi + 2], sizeof(Seg) * (k.ns - bi - 2));
        k.ns--;
    }
    if (bi > 0 && k.seg[bi - 1].y == k.seg[bi].y) {
        k.seg[bi - 1].w += k.seg[bi].w;
        memmove(&k.seg[bi], &k.seg[bi + 1], sizeof(Seg) * (k.ns - bi - 1));
        k.ns--;
    }
}

// Korf-style wasted-space lower bound for the free region above the skyline (up to dH).
// Rows: maximal free intervals of width w can only take squares of side <= w. Same for columns.
static inline long long relaxWaste(const long long* cap) {
    long long waste = 0, carry = 0;
    for (int w = 1; w <= MS; w++) {
        carry += (long long)dCnt[w] * w * w;
        long long used = min(cap[w], carry);
        waste += cap[w] - used;
        carry -= used;
    }
    return waste;
}
static inline long long wasteLB(const Sky& k) {
    long long capR[32] = {0}, capC[32] = {0};
    int stx[MAXSEG + 1], sth[MAXSEG + 1], sp = 0;
    int x = 0;
    for (int i = 0; i <= k.ns; i++) {
        int h = i < k.ns ? max(0, dH - k.seg[i].y) : 0;
        int w = i < k.ns ? k.seg[i].w : 0;
        if (i < k.ns && h > 0) capC[min(h, MS)] += (long long)w * h;
        int start = x;
        while (sp > 0 && sth[sp - 1] > h) {
            int sx = stx[sp - 1], sh = sth[sp - 1];
            sp--;
            int below = max(h, sp > 0 ? sth[sp - 1] : 0);
            int width = x - sx;
            capR[min(width, MS)] += (long long)(sh - below) * width;
            start = sx;
        }
        if (h > 0 && (sp == 0 || sth[sp - 1] < h)) { stx[sp] = start; sth[sp] = h; sp++; }
        x += w;
    }
    return max(relaxWaste(capR), relaxWaste(capC));
}

static bool dfs(int d) {
    if (dLeft == 0) return true;
    if (++dNodes > dLimit) { dAbort = true; return false; }
    if ((dNodes & 4095) == 0 && chrono::duration<double>(chrono::steady_clock::now() - gT0).count() > gcfg.time_budget_s) {
        dAbort = true; return false;
    }
    if (d + 1 >= MAXDEPTH) return false;
    const Sky& k = stk[d];
    int bi = 0;
    for (int i = 1; i < k.ns; i++)
        if (k.seg[i].y < k.seg[bi].y) bi = i;
    const Seg g = k.seg[bi];
    if (g.y >= dH) return false;
    if (dWaste + wasteLB(k) > dBudget) return false;

    int lim = min(min(g.w, dH - g.y), MS);
    int ly = bi > 0 ? k.seg[bi - 1].y : INT_MAX;
    int ry = bi + 1 < k.ns ? k.seg[bi + 1].y : INT_MAX;
    bool placeLeft = ly >= ry;

    int cands[33], nc = 0;
    for (int s = lim; s >= 1; s--) if (dCnt[s]) cands[nc++] = s;
    // option "raise" encoded as 0
    int ny = min(min(ly, ry), dH);
    bool canRaise = min(ly, ry) != INT_MAX && dWaste + (long long)g.w * (ny - g.y) <= dBudget;
    if (canRaise) cands[nc++] = 0;
    if (nc > 1 && urand() < gcfg.p_shuffle) {
        int r = 1 + rng() % (nc - 1);
        rotate(cands, cands + r, cands + nc);
    }
    for (int ci = 0; ci < nc; ci++) {
        int s = cands[ci];
        Sky& c = stk[d + 1];
        c.ns = k.ns;
        memcpy(c.seg, k.seg, sizeof(Seg) * k.ns);
        if (s == 0) {
            long long w = (long long)g.w * (ny - g.y);
            c.seg[bi].y = ny;
            skyMerge(c, bi);
            dWaste += w;
            bool ok = dfs(d + 1);
            dWaste -= w;
            if (ok) return true;
        } else {
            int b = bi;
            int px = placeLeft ? g.x : g.x + g.w - s;
            if (s == g.w) {
                c.seg[b].y += s;
            } else {
                memmove(&c.seg[b + 2], &c.seg[b + 1], sizeof(Seg) * (c.ns - b - 1));
                c.ns++;
                if (placeLeft) {
                    c.seg[b] = {g.x, s, g.y + s};
                    c.seg[b + 1] = {g.x + s, g.w - s, g.y};
                } else {
                    c.seg[b] = {g.x, g.w - s, g.y};
                    c.seg[b + 1] = {g.x + g.w - s, s, g.y + s};
                    b++;
                }
            }
            skyMerge(c, b);
            dCnt[s]--; dLeft--;
            dPath.push_back({s, px, g.y});
            bool ok = dfs(d + 1);
            if (ok) return true;
            dPath.pop_back();
            dCnt[s]++; dLeft++;
        }
        if (dAbort) return false;
    }
    return false;
}

static bool runDfs(int W, int H, long long limit, vector<Place>& out) {
    dW = W; dH = H;
    dBudget = (long long)W * H - S;
    dWaste = 0; dNodes = 0; dLimit = limit; dAbort = false;
    memcpy(dCnt, cnt0, sizeof dCnt);
    dLeft = n;
    dPath.clear();
    stk[0].ns = 1;
    stk[0].seg[0] = {0, W, 0};
    bool ok = dfs(0);
    if (ok) out = dPath;
    return ok;
}

int main(int argc, char** argv) {
    Config cfg;
    if (argc > 1) cfg.time_budget_s = atof(argv[1]);   // offline experiments only
    for (int a = 2; a < argc; a++) {                     // key=value overrides (offline only)
        string kv = argv[a]; auto eq = kv.find('='); string k = kv.substr(0, eq); double v = atof(kv.c_str() + eq + 1);
        if (k == "p_shuffle") cfg.p_shuffle = v;
        else if (k == "limit_growth") cfg.limit_growth = v;
        else if (k == "dfs_nodes0") cfg.dfs_nodes0 = (long long)v;
        else if (k == "phase1_frac") cfg.phase1_frac = v;
        else if (k == "max_aspect") cfg.max_aspect = v;
        else if (k == "p_not_largest") cfg.p_not_largest = v;
        else if (k == "p_flip_side") cfg.p_flip_side = v;
        else if (k == "seed") cfg.seed = (uint64_t)v;
    }
    gcfg = cfg;
    rng.seed(cfg.seed);
    auto t0 = chrono::steady_clock::now();
    gT0 = t0;
    auto elapsed = [&] { return chrono::duration<double>(chrono::steady_clock::now() - t0).count(); };

    if (!(cin >> n)) return 0;
    side.resize(n);
    S = 0;
    MS = 0;
    memset(cnt0, 0, sizeof cnt0);
    for (auto& s : side) { cin >> s; S += (long long)s * s; MS = max(MS, s); cnt0[s]++; }

    const int BIG = INT_MAX / 2;
    long long bestA = LLONG_MAX;
    vector<Place> best, tmp;
    int bestW = 0, bestH = 0;
    int maxW = max<long long>(MS, S / MS + MS);
    for (int W = MS; W <= maxW; W++) {
        int top;
        decodeRand(W, BIG, cfg, false, tmp, top);
        long long A = (long long)W * top;
        if (A < bestA) { bestA = A; best = tmp; bestW = W; bestH = top; }
    }
    long long initA = bestA;

    auto buildTargets = [&](vector<pair<long long, pair<int,int>>>& targets) {
        targets.clear();
        for (int W = MS; W <= maxW; W++) {
            int H0 = max<long long>(MS, (S + W - 1) / W);
            for (int H = H0; (long long)W * H < bestA; H++) {
                if (max(W, H) > cfg.max_aspect * min(W, H)) continue;
                targets.push_back({(long long)W * H, {W, H}});
            }
        }
        sort(targets.begin(), targets.end());
    };

    // ---- Phase 1 ----
    long long iters = 0;
    vector<pair<long long, pair<int,int>>> targets;
    double p1end = cfg.time_budget_s * cfg.phase1_frac;
    while (elapsed() < p1end) {
        buildTargets(targets);
        if (targets.empty()) break;
        bool improved = false;
        for (auto& t : targets) {
            int W = t.second.first, H = t.second.second;
            for (int k = 0; k < cfg.tries_per_target; k++) {
                int top;
                iters++;
                if (decodeRand(W, H, cfg, true, tmp, top)) {
                    bestA = t.first; best = tmp; bestW = W; bestH = H;
                    improved = true;
                    break;
                }
            }
            if (improved || elapsed() > p1end) break;
        }
    }
    long long p1A = bestA;

    // ---- Phase 2: DFS, smallest area first, node limit doubles per full pass ----
    long long limit = cfg.dfs_nodes0, totalNodes = 0;
    int passes = 0;
    while (elapsed() < cfg.time_budget_s) {
        buildTargets(targets);
        if (targets.empty()) break;
        bool improved = false;
        for (auto& t : targets) {
            int W = t.second.first, H = t.second.second;
            bool ok = runDfs(W, H, limit, tmp);
            totalNodes += dNodes;
            if (ok) {
                bestA = t.first; best = tmp; bestW = W; bestH = H;
                improved = true;
                break;
            }
            if (elapsed() > cfg.time_budget_s) break;
        }
        if (!improved) { limit = min((long long)(limit * cfg.limit_growth), (long long)4e9); passes++; }
    }
    cerr << "init " << initA << " p1 " << p1A << " final " << bestA << " (" << bestW << "x" << bestH
         << ") S=" << S << " p1iters=" << iters << " dfsNodes=" << totalNodes << " limit=" << limit << "\n";

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
