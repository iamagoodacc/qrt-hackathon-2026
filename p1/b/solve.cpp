// p1-b: square packing by fixed-container skyline DFS (port of best/p0 solve4, generalised).
// For a container W x H, fill the lowest-leftmost skyline gap with a remaining square
// (distinct sizes, ordered by a fit heuristic + random noise) or waste the gap up to its
// lower neighbour. Prune when wasted area exceeds W*H - S. Randomised restarts with node
// limits; shrink the target area while time remains.
// Generalised: segment skyline (any side size), sampled widths, long long coordinates.
#include <bits/stdc++.h>
using namespace std;

namespace P1CFG {
double TIME_BUDGET_S = 1.5;      // total search time inside pack_squares
double PHASE1_FRAC = 0.25;       // share of the budget for the greedy strip phase
double START_NODES = 2000;       // node limit per DFS attempt in the first round
double NODE_GROWTH = 2.0;        // node limit multiplier after a round without improvement
double MAX_NODES = 50000;        // node limit cap; beyond it we just keep restarting
double BONUS_EXACT = 100;        // key bonus (side units at m=30): square exactly fills the gap
double BONUS_LEVEL = 50;         // key bonus: square top is level with the placing-side neighbour
double NOISE = 4.0;              // random key noise (side units at m=30) on restarts
int TALLER_SIDE = 1;             // 1 = place against the taller neighbour, 0 = always left
long long MAX_WIDTHS = 4000;     // max widths tried per phase (sampled evenly if more)
}

namespace p1b {
using ll = long long;
struct Seg { ll x, w, h; };

chrono::steady_clock::time_point T0;
double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }
uint64_t rngState;
inline double urand() {
    rngState ^= rngState << 7; rngState ^= rngState >> 9;
    return (rngState >> 11) * (1.0 / 9007199254740992.0);
}

int n, K;                 // K distinct sizes
ll m, S;
vector<ll> sizes;         // distinct sizes, descending
vector<int> sizeIdx;      // input i -> distinct index
vector<int> cntInit, cnt;
double scale;

ll W, H, budget, wasted;
vector<vector<Seg>> skyStack;  // skyline per depth
int remaining;
ll nodes, nodeLimit;
bool aborted;
double noise;
vector<array<ll, 3>> placed;   // (size idx, row, col)

bool dfs(int d) {
    if (remaining == 0) return true;
    if (++nodes > nodeLimit) { aborted = true; return false; }
    if ((int)skyStack.size() <= d + 1) skyStack.resize(d + 2);
    const vector<Seg> &sky = skyStack[d];
    int i = 0;
    for (int j = 1; j < (int)sky.size(); j++) if (sky[j].h < sky[i].h) i = j;
    ll x = sky[i].x, g = sky[i].w, h0 = sky[i].h;
    ll leftH = i > 0 ? sky[i - 1].h : H;
    ll rightH = i + 1 < (int)sky.size() ? sky[i + 1].h : H;
    ll maxS = min(g, H - h0);
    bool right = P1CFG::TALLER_SIDE && rightH > leftH;
    ll sideH = right ? rightH : leftH;
    // candidates
    static thread_local vector<vector<pair<double, int>>> candStack;
    if ((int)candStack.size() <= d) candStack.resize(d + 1);
    vector<pair<double, int>> &cand = candStack[d];
    cand.clear();
    for (int k = 0; k < K; k++) {
        if (!cnt[k] || sizes[k] > maxS) continue;
        ll s = sizes[k];
        double key = (double)s / scale;
        if (s == g) key += P1CFG::BONUS_EXACT;
        if (h0 + s == sideH) key += P1CFG::BONUS_LEVEL;
        if (noise > 0) key += noise * urand();
        cand.push_back({key, k});
    }
    sort(cand.begin(), cand.end(), [](auto &a, auto &b) { return a.first > b.first; });
    for (size_t c = 0; c < cand.size(); c++) {
        int k = cand[c].second;
        ll s = sizes[k];
        cnt[k]--; remaining--;
        ll px = right ? x + g - s : x;
        // build next skyline
        vector<Seg> &nx = skyStack[d + 1];
        const vector<Seg> &cur = skyStack[d];
        nx.clear();
        for (int j = 0; j < (int)cur.size(); j++) {
            if (j != i) { nx.push_back(cur[j]); continue; }
            if (s == g) nx.push_back({x, g, h0 + s});
            else if (right) { nx.push_back({x, g - s, h0}); nx.push_back({px, s, h0 + s}); }
            else { nx.push_back({x, s, h0 + s}); nx.push_back({x + s, g - s, h0}); }
        }
        // merge equal neighbours
        int w = 0;
        for (int j = 0; j < (int)nx.size(); j++) {
            if (w > 0 && nx[w - 1].h == nx[j].h) nx[w - 1].w += nx[j].w;
            else nx[w++] = nx[j];
        }
        nx.resize(w);
        placed.push_back({k, h0, px});
        if (dfs(d + 1)) return true;
        placed.pop_back();
        cnt[k]++; remaining++;
        if (aborted) return false;
    }
    // waste the gap up to its lower neighbour
    ll nh = min(leftH, rightH);
    if (nh <= h0 || nh - h0 > (budget - wasted) / g) return false;
    ll wst = g * (nh - h0);
    wasted += wst;
    {
        vector<Seg> &nx = skyStack[d + 1];
        const vector<Seg> &cur = skyStack[d];
        nx.clear();
        for (int j = 0; j < (int)cur.size(); j++) {
            Seg sg = cur[j];
            if (j == i) sg.h = nh;
            if (!nx.empty() && nx.back().h == sg.h) nx.back().w += sg.w;
            else nx.push_back(sg);
        }
    }
    bool ok = dfs(d + 1);
    if (ok) return true;
    wasted -= wst;
    return false;
}

// Try to pack everything into w x h. On success fills `out` (row, col per input index).
bool tryPack(ll w, ll h, ll limit, double nz, vector<pair<ll, ll>> &out, ll &area) {
    W = w; H = h;
    budget = (h > (ll)4e18 / w) ? (ll)4e18 : w * h - S;
    if (budget < 0) return false;
    wasted = 0;
    if (skyStack.empty()) skyStack.resize(1);
    skyStack[0].assign(1, Seg{0, W, 0});
    cnt = cntInit;
    remaining = n;
    nodes = 0; nodeLimit = limit; aborted = false; noise = nz;
    placed.clear();
    if (!dfs(0)) return false;
    vector<vector<int>> bySize(K);
    for (int i = 0; i < n; i++) bySize[sizeIdx[i]].push_back(i);
    out.assign(n, {0, 0});
    ll maxRow = 0, maxCol = 0;
    for (auto &p : placed) {
        int i = bySize[p[0]].back(); bySize[p[0]].pop_back();
        out[i] = {p[1], p[2]};
        maxRow = max(maxRow, p[1] + sizes[p[0]]);
        maxCol = max(maxCol, p[2] + sizes[p[0]]);
    }
    area = maxRow * maxCol;
    return true;
}

ll lowerBound() {
    ll best = m * max(m, (S + m - 1) / m);
    ll w = m, last = (ll)sqrtl((long double)S) + 2;
    while (w <= last) {
        ll h = max(m, (S + w - 1) / w);
        best = min(best, w * h);
        if (h <= m) break;
        w = max(w + 1, (S - 1) / (h - 1) + 1);
    }
    return best;
}

// widths in [lo, hi], all of them if few, else evenly sampled
vector<ll> widthList(ll lo, ll hi) {
    vector<ll> ws;
    if (hi < lo) return ws;
    ll cntW = hi - lo + 1;
    if (cntW <= P1CFG::MAX_WIDTHS) { for (ll w = lo; w <= hi; w++) ws.push_back(w); }
    else {
        for (ll t = 0; t < P1CFG::MAX_WIDTHS; t++)
            ws.push_back(lo + (ll)((long double)(hi - lo) * t / (P1CFG::MAX_WIDTHS - 1)));
        ws.erase(unique(ws.begin(), ws.end()), ws.end());
    }
    return ws;
}

vector<vector<int>> solve(const vector<int> &squares) {
    T0 = chrono::steady_clock::now();
    rngState = 88172645463325252ULL;
    n = squares.size();
    vector<vector<int>> res(n, vector<int>(2, 0));
    if (n == 0) return res;
    m = 0; S = 0;
    for (int s : squares) { m = max<ll>(m, s); S += (ll)s * s; }
    sizes.assign(squares.begin(), squares.end());
    sort(sizes.rbegin(), sizes.rend());
    sizes.erase(unique(sizes.begin(), sizes.end()), sizes.end());
    K = sizes.size();
    cntInit.assign(K, 0);
    sizeIdx.assign(n, 0);
    for (int i = 0; i < n; i++) {
        sizeIdx[i] = lower_bound(sizes.begin(), sizes.end(), (ll)squares[i], greater<ll>()) - sizes.begin();
        cntInit[sizeIdx[i]]++;
    }
    scale = max(1.0, (double)m / 30.0);
    skyStack.clear(); placed.clear();
    ll L = lowerBound();

    // Safe fallback: all squares in one row.
    vector<pair<ll, ll>> best(n), cur;
    {
        ll y = 0;
        for (int i = 0; i < n; i++) { best[i] = {0, y}; y += squares[i]; }
    }
    ll bestArea = LLONG_MAX, area;
    {
        long double a = (long double)m;
        long double sumS = 0; for (int s : squares) sumS += s;
        if (a * sumS < 9e18) bestArea = m * (ll)sumS;
    }

    // Phase 1: greedy (unbounded height) for many widths.
    ll maxW = min<ll>(S / m + m, (ll)2e9);
    const ll INF_H = (ll)2e18;
    for (ll w : widthList(m, maxW)) {
        if (tryPack(w, INF_H, LLONG_MAX, 0, cur, area) && area < bestArea) { bestArea = area; best = cur; }
        if (bestArea <= L || elapsed() > P1CFG::TIME_BUDGET_S * P1CFG::PHASE1_FRAC) break;
    }

    // Phase 2: containers with area below the best, most slack first.
    double limit = P1CFG::START_NODES;
    int round = 0;
    while (bestArea > L && elapsed() < P1CFG::TIME_BUDGET_S) {
        ll target = bestArea - 1;
        vector<pair<ll, ll>> cands;  // (container area, width)
        for (ll w : widthList(m, target / m)) {
            ll h = target / w;
            if (h < m || w * h < S) continue;
            cands.push_back({w * h, w});
        }
        if (cands.empty()) break;
        sort(cands.rbegin(), cands.rend());
        bool improved = false;
        for (auto &[a, w] : cands) {
            if (elapsed() > P1CFG::TIME_BUDGET_S) break;
            if (a >= bestArea) continue;
            bool ok = tryPack(w, a / w, (ll)limit, round == 0 ? 0 : P1CFG::NOISE, cur, area);
            if (ok && area < bestArea) { bestArea = area; best = cur; improved = true; break; }
        }
        if (!improved) { limit = min(limit * P1CFG::NODE_GROWTH, P1CFG::MAX_NODES); round++; }
    }
#ifdef LOCAL
    cerr << "area " << bestArea << " L " << L << " ratio " << (double)L / bestArea
         << " rounds " << round << " t " << elapsed() << "\n";
#endif
    for (int i = 0; i < n; i++) { res[i][0] = (int)best[i].first; res[i][1] = (int)best[i].second; }
    return res;
}
}  // namespace p1b

vector<vector<int>> pack_squares(vector<int> squares) { return p1b::solve(squares); }

#ifdef LOCAL
int main() {
    // local-only tuning overrides via environment variables
    if (getenv("TB")) P1CFG::TIME_BUDGET_S = atof(getenv("TB"));
    if (getenv("NOISE")) P1CFG::NOISE = atof(getenv("NOISE"));
    if (getenv("MAXN")) P1CFG::MAX_NODES = atof(getenv("MAXN"));
    if (getenv("STARTN")) P1CFG::START_NODES = atof(getenv("STARTN"));
    int n;
    if (!(cin >> n)) return 0;
    vector<int> a(n);
    for (int &x : a) cin >> x;
    auto r = pack_squares(a);
    string out;
    for (auto &p : r) out += to_string(p[0]) + " " + to_string(p[1]) + "\n";
    cout << out;
}
#endif
