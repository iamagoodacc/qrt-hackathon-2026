// p1-c: square packing by BEAM SEARCH over skyline states in a fixed W x H container.
// Move = fill the lowest-leftmost skyline gap with a remaining square (against the taller
// neighbour) or waste the gap up to its lower neighbour. Beam keeps the B best states per layer
// (key = waste incl. forced dead-sliver waste, minus progress), deduplicated by hash.
// Containers with area below the best are tried, most slack first; beam width grows on failure.
#include <bits/stdc++.h>
using namespace std;

namespace CFG {
double TIME_BUDGET_S = 1.5;      // total time per call
int BEAM_START = 8;              // beam width in the first container round
int BEAM_MAX = 512;              // beam width cap
double BEAM_GROWTH = 2.0;        // multiply beam width after a round with no improvement
double W_WASTE = 1.0;            // key weight of (waste + forced waste)
double W_AREA = 0.02;            // key bonus per unit of placed area (progress / big-first)
double W_EXACT = 3.0;            // key bonus (in area units) for a square exactly filling the gap
double W_LEVEL = 2.0;            // key bonus for a square level with its neighbour
double NOISE = 0.5;              // random key noise (area units) after the first round
int GREEDY_WIDTHS = 4000;        // max widths tried by the initial greedy strip packing
}

namespace pk {
using ll = long long;
chrono::steady_clock::time_point T0;
double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }
uint64_t rs = 88172645463325252ULL;
inline double urand() { rs ^= rs << 7; rs ^= rs >> 9; return (rs >> 11) * (1.0 / 9007199254740992.0); }

struct Seg { int x, w, h; };
struct State {
    vector<Seg> sky;
    vector<int> cnt;
    ll waste, placed;
    int node;
    int remaining;
};
struct Node { int parent, k, row, col; };

int K;                 // distinct sizes, sz[] descending
vector<int> sz, cnt0;
ll S;
int n, m;
vector<Node> nodes;

inline int lowest(const vector<Seg> &sky) {
    int b = 0;
    for (int i = 1; i < (int)sky.size(); i++) if (sky[i].h < sky[b].h) b = i;
    return b;
}
inline void mergeAround(vector<Seg> &sky, int i) {
    if (i + 1 < (int)sky.size() && sky[i + 1].h == sky[i].h) { sky[i].w += sky[i + 1].w; sky.erase(sky.begin() + i + 1); }
    if (i > 0 && sky[i - 1].h == sky[i].h) { sky[i - 1].w += sky[i].w; sky.erase(sky.begin() + i); }
}
// place square s at gap i, right side if `right`; returns column
inline int placeSq(vector<Seg> &sky, int i, int s, bool right) {
    Seg g = sky[i];
    if (s == g.w) { sky[i].h += s; mergeAround(sky, i); return g.x; }
    if (!right) {
        sky[i] = {g.x + s, g.w - s, g.h};
        sky.insert(sky.begin() + i, Seg{g.x, s, g.h + s});
        mergeAround(sky, i);
        return g.x;
    }
    sky[i] = {g.x, g.w - s, g.h};
    sky.insert(sky.begin() + i + 1, Seg{g.x + g.w - s, s, g.h + s});
    mergeAround(sky, i + 1);
    return g.x + g.w - s;
}

uint64_t hashState(const State &st) {
    uint64_t h = 1469598103934665603ULL;
    auto mix = [&](uint64_t v) { h ^= v + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2); };
    for (auto &s : st.sky) { mix((uint64_t)s.x << 32 | (uint32_t)s.w); mix(s.h); }
    for (int c : st.cnt) mix(c);
    return h;
}

struct Cand { double key; int st, k; bool right; ll addWaste; };

// Beam search in W x H container. Returns true and fills out (row,col per placed node chain).
bool beam(int W, int H, int B, double noise, vector<array<int, 3>> &placedOut) {
    ll budget = (ll)W * H - S;
    if (budget < 0 || W < m || H < m) return false;
    nodes.clear();
    vector<State> cur(1), nxt;
    cur[0].sky = {Seg{0, W, 0}};
    cur[0].cnt = cnt0;
    cur[0].waste = 0; cur[0].placed = 0; cur[0].node = -1; cur[0].remaining = n;
    vector<Cand> cands;
    unordered_set<uint64_t> seen;
    while (true) {
        cands.clear();
        for (int si = 0; si < (int)cur.size(); si++) {
            State &st = cur[si];
            if (st.remaining == 0) {
                placedOut.clear();
                for (int v = st.node; v >= 0; v = nodes[v].parent)
                    if (nodes[v].k >= 0) placedOut.push_back({nodes[v].k, nodes[v].row, nodes[v].col});
                return true;
            }
            int i = lowest(st.sky);
            const Seg &g = st.sky[i];
            int leftH = i > 0 ? st.sky[i - 1].h : H;
            int rightH = i + 1 < (int)st.sky.size() ? st.sky[i + 1].h : H;
            bool right = rightH > leftH;
            int tallH = max(leftH, rightH), shortH = min(leftH, rightH);
            int room = H - g.h;
            // smallest remaining size (sz descending -> last k with cnt)
            int minIdx = -1;
            for (int k = K - 1; k >= 0; k--) if (st.cnt[k]) { minIdx = k; break; }
            for (int k = 0; k < K; k++) {
                if (!st.cnt[k]) continue;
                int s = sz[k];
                if (s > g.w || s > room) continue;
                ll forced = 0;
                if (s < g.w) {
                    // leftover sliver width g.w - s at height g.h, between square (g.h+s) and short neighbour
                    int minRem = sz[minIdx];
                    if (k == minIdx && st.cnt[k] == 1) {
                        minRem = INT_MAX;
                        for (int k2 = K - 1; k2 >= 0; k2--) if (st.cnt[k2] && k2 != k) { minRem = sz[k2]; break; }
                    }
                    if (g.w - s < minRem) forced = (ll)(g.w - s) * (min(g.h + s, shortH) - g.h);
                }
                if (st.waste + forced > budget) continue;
                double key = CFG::W_WASTE * (st.waste + forced) - CFG::W_AREA * (st.placed + (ll)s * s);
                if (s == g.w) key -= CFG::W_EXACT;
                if (g.h + s == tallH) key -= CFG::W_LEVEL;
                if (noise > 0) key += noise * urand();
                cands.push_back({key, si, k, right, 0});
            }
            // waste move
            int nh = min(leftH, rightH);
            ll w = (ll)g.w * (nh - g.h);
            if (w > 0 && st.waste + w <= budget) {
                double key = CFG::W_WASTE * (st.waste + w) - CFG::W_AREA * st.placed;
                if (noise > 0) key += noise * urand();
                cands.push_back({key, si, -1, false, w});
            }
        }
        if (cands.empty()) return false;
        sort(cands.begin(), cands.end(), [](const Cand &a, const Cand &b) { return a.key < b.key; });
        nxt.clear();
        seen.clear();
        for (auto &c : cands) {
            if ((int)nxt.size() >= B) break;
            const State &p = cur[c.st];
            State ch;
            ch.sky = p.sky; ch.cnt = p.cnt; ch.waste = p.waste; ch.placed = p.placed; ch.remaining = p.remaining;
            int i = lowest(ch.sky);
            int row = ch.sky[i].h;
            if (c.k >= 0) {
                int s = sz[c.k];
                int col = placeSq(ch.sky, i, s, c.right);
                ch.cnt[c.k]--; ch.remaining--; ch.placed += (ll)s * s;
                nodes.push_back({p.node, c.k, row, col});
            } else {
                int leftH = i > 0 ? ch.sky[i - 1].h : H;
                int rightH = i + 1 < (int)ch.sky.size() ? ch.sky[i + 1].h : H;
                ch.sky[i].h = min(leftH, rightH);
                ch.waste += c.addWaste;
                mergeAround(ch.sky, i);
                nodes.push_back({p.node, -1, 0, 0});
            }
            ch.node = (int)nodes.size() - 1;
            if (!seen.insert(hashState(ch)).second) { nodes.pop_back(); continue; }
            nxt.push_back(move(ch));
        }
        swap(cur, nxt);
        if (elapsed() > CFG::TIME_BUDGET_S) return false;
    }
}

// Greedy strip packing (beam width 1, unbounded height) for width W; returns area and placement.
}  // namespace pk

vector<vector<int>> pack_squares(vector<int> squares) {
    using namespace pk;
    T0 = chrono::steady_clock::now();
    n = (int)squares.size();
    vector<vector<int>> res(n, vector<int>(2, 0));
    if (n == 0) return res;
    m = 0; S = 0;
    for (int s : squares) { m = max(m, s); S += (ll)s * s; }
    // distinct sizes descending
    sz = squares; sort(sz.rbegin(), sz.rend()); sz.erase(unique(sz.begin(), sz.end()), sz.end());
    K = (int)sz.size();
    cnt0.assign(K, 0);
    vector<vector<int>> byK(K);
    for (int i = 0; i < n; i++) {
        int k = int(lower_bound(sz.begin(), sz.end(), squares[i], greater<int>()) - sz.begin());
        cnt0[k]++; byK[k].push_back(i);
    }
    // ---- fallback: one row (always valid) ----
    vector<vector<int>> best(n, vector<int>(2, 0));
    ll bestArea;
    {
        ll col = 0;
        for (int i = 0; i < n; i++) { best[i] = {0, (int)col}; col += squares[i]; }
        bestArea = (ll)m * col;
    }
    auto adopt = [&](vector<array<int, 3>> &pl) {
        vector<vector<int>> idx = byK;
        vector<vector<int>> out(n, vector<int>(2));
        ll R = 0, C = 0;
        for (auto &p : pl) {
            int i = idx[p[0]].back(); idx[p[0]].pop_back();
            out[i] = {p[1], p[2]};
            R = max(R, (ll)p[1] + sz[p[0]]); C = max(C, (ll)p[2] + sz[p[0]]);
        }
        if (R * C < bestArea) { bestArea = R * C; best = out; return true; }
        return false;
    };
    vector<array<int, 3>> pl;
    // ---- phase 1: greedy strip packing (beam 1, tall container) for many widths ----
    {
        ll hiW = min<ll>((ll)m + S / m, (ll)INT_MAX / 2);
        ll span = hiW - m + 1;
        int steps = (int)min<ll>(span, CFG::GREEDY_WIDTHS);
        for (int t = 0; t < steps; t++) {
            ll W = m + (span <= CFG::GREEDY_WIDTHS ? t : span * t / steps);
            ll Hcap = min<ll>((ll)INT_MAX / 2, max<ll>((ll)m, (bestArea + W - 1) / W + m));
            if (beam((int)W, (int)Hcap, 1, 0, pl)) adopt(pl);
            if (elapsed() > CFG::TIME_BUDGET_S * 0.2) break;
        }
    }
    // ---- phase 2: containers below the best area, most slack first; beam grows on failure ----
    double B = CFG::BEAM_START;
    int round = 0;
    while (elapsed() < CFG::TIME_BUDGET_S) {
        ll target = bestArea - 1;
        vector<pair<ll, int>> cands;
        for (ll W = m; W * m <= target && W <= INT_MAX / 2; W++) {
            ll H = target / W;
            if (H < m || W * H < S || H > INT_MAX / 2) continue;
            cands.push_back({W * H, (int)W});
            if (cands.size() > 200000) break;
        }
        if (cands.empty()) break;
        sort(cands.rbegin(), cands.rend());
        bool improved = false;
        for (auto &[a, W] : cands) {
            if (elapsed() > CFG::TIME_BUDGET_S) break;
            if (a >= bestArea) continue;
            if (beam(W, (int)(a / W), (int)B, round == 0 ? 0 : CFG::NOISE, pl) && adopt(pl)) { improved = true; break; }
        }
        if (!improved) { B = min(B * CFG::BEAM_GROWTH, (double)CFG::BEAM_MAX); round++; }
    }
    // normalise to min coord 0 (already) and return
    return best;
}

int main() {
    int n;
    if (!(cin >> n)) return 0;
    vector<int> a(n);
    for (auto &x : a) cin >> x;
    auto r = pack_squares(a);
    for (auto &p : r) cout << p[0] << ' ' << p[1] << '\n';
}
