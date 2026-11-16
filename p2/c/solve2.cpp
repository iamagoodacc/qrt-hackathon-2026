// p2-c: word search generation on an N x N torus.
// Constructive: randomised max-overlap greedy (each step looks at the next K unplaced words in a
// noisy longest-first order and places the one whose best placement overlaps most), many restarts
// per N, shrink N while it succeeds. Blank cells get letters that never complete a forbidden word.
#include <bits/stdc++.h>
using namespace std;

namespace CFG2 {
double TIME_BUDGET_S = 1.5;      // total time per call
double OVERLAP_W = 1.0;          // score per overlapping (shared) cell
double CONTACT_W = 0.3;          // score per filled cell just before/after the word on its line
double NOISE = 0.6;              // random noise added to placement scores (tie-breaks / diversity)
double ORDER_NOISE = 3.0;        // random noise on word length for the placement order
int CANDS_K = 4;                 // unplaced words examined per greedy step
int FIRST_TRIES = 1;             // attempts per N while searching upward for a feasible N
double FIRST_LNS_S = 0.1;        // LNS time slice per N while searching upward for a feasible N
double LNS_SLICE_S = 0.25;       // LNS restart period when shrinking N
int GREEDY_PER_ROUND = 2;        // greedy attempts before each LNS slice when shrinking N
double WEIGHT_INC = 1.0;         // LNS: weight added to a word each time it is ejected
double EJECT_W = 1.0;            // LNS cost per letter of an ejected word
double TABU = 3;                 // LNS: a word placed within this many iterations is tabu
double TABU_COST = 1000;         // LNS: cost of ejecting a tabu word
double LNS_RANDOM_PICK = 0.1;    // LNS: chance to pick a random pending word instead of the longest
}

namespace wsg {
using ll = long long;
chrono::steady_clock::time_point T0;
double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }
uint64_t rs = 0x9E3779B97F4A7C15ULL;
inline uint64_t rnd() { rs ^= rs << 7; rs ^= rs >> 9; return rs; }
inline double urand() { return (rnd() >> 11) * (1.0 / 9007199254740992.0); }

const int DR[8] = {0, 0, 1, -1, 1, -1, 1, -1};
const int DC[8] = {1, -1, 0, 0, 1, -1, -1, 1};

int N, LM;                 // grid side, max word length used for tables (incl. forbidden, capped)
vector<string> req, forb;
vector<char> grid;         // 0 = empty
vector<int> step;          // step[(d*N*N + cell)] = next cell in direction d
bool timeUp = false;

inline int nxt(int id, int d) { return step[d * N * N + id]; }
void buildTables() {
    step.assign(8 * N * N, 0);
    for (int d = 0; d < 8; d++) for (int r = 0; r < N; r++) for (int c = 0; c < N; c++)
        step[d * N * N + r * N + c] = ((r + DR[d] + N) % N) * N + (c + DC[d] + N) % N;
}
// does word w appear (fully) starting at id in direction d (cyclic, self-wrap allowed)?
inline bool wordAt(const string &w, int id, int d) {
    for (size_t k = 0; k < w.size(); k++) { if (grid[id] != w[k]) return false; id = nxt(id, d); }
    return true;
}
// any forbidden word fully present that passes through cell id?
bool forbThrough(int id0) {
    char g0 = grid[id0];
    for (auto &f : forb) {
        int L = f.size();
        for (int k = 0; k < L; k++) {
            if (f[k] != g0) continue;
            for (int d = 0; d < 8; d++) {
                // start = id0 moved back k steps in d  (= forward k steps in opposite dir d^1)
                int st = id0;
                for (int t = 0; t < k; t++) st = nxt(st, d ^ 1);
                if (wordAt(f, st, d)) return true;
            }
        }
    }
    return false;
}
bool present(const string &w) {
    for (int id = 0; id < N * N; id++) if (grid[id] == w[0])
        for (int d = 0; d < 8; d++) if (wordAt(w, id, d)) return true;
    return false;
}

vector<int> safeLetters;
bool fillBlanks() {
    for (int i = 0; i < N * N; i++) {
        if (grid[i]) continue;
        if (!safeLetters.empty()) { grid[i] = 'A' + safeLetters[rnd() % safeLetters.size()]; continue; }
        int start = rnd() % 26; bool ok = false;
        for (int t = 0; t < 26 && !ok; t++) {
            grid[i] = 'A' + (start + t) % 26;
            if (!forbThrough(i)) ok = true;
        }
        if (!ok) return false;
    }
    return true;
}

// One randomised greedy attempt at size N.
bool attempt() {
    grid.assign(N * N, 0);
    int R = req.size();
    vector<pair<double, int>> ord(R);
    for (int i = 0; i < R; i++) ord[i] = {(double)req[i].size() + CFG2::ORDER_NOISE * urand(), i};
    sort(ord.rbegin(), ord.rend());
    vector<int> pending;
    for (auto &p : ord) pending.push_back(p.second);
    vector<int> cells(64);
    while (!pending.empty()) {
        if (elapsed() > CFG2::TIME_BUDGET_S) { timeUp = true; return false; }  // (attempt)
        double bestSc = -1e18; int bwPos = -1, bId = 0, bD = 0;
        int K = min<int>(CFG2::CANDS_K, pending.size());
        for (int pi = 0; pi < K; pi++) {
            const string &w = req[pending[pi]];
            int L = w.size();
            double wBest = -1e18; int wId = 0, wD = 0; bool already = false;
            for (int id = 0; id < N * N && !already; id++) {
                char g0 = grid[id];
                if (g0 && g0 != w[0]) continue;
                for (int d = 0; d < 8; d++) {
                    int ov = 0, cur = id; bool okp = true;
                    for (int k = 0; k < L; k++) {
                        char g = grid[cur];
                        if (g) { if (g != w[k]) { okp = false; break; } ov++; }
                        cur = nxt(cur, d);
                    }
                    if (!okp) continue;
                    if (ov == L) { already = true; break; }
                    // contacts: filled cell right after the end and right before the start
                    int contact = (grid[cur] != 0) + (grid[nxt(id, d ^ 1)] != 0);
                    double sc = CFG2::OVERLAP_W * ov + CFG2::CONTACT_W * contact + CFG2::NOISE * urand();
                    if (sc > wBest) { wBest = sc; wId = id; wD = d; }
                }
            }
            if (already) { pending.erase(pending.begin() + pi); pi--; K = min<int>(K, pending.size()); continue; }
            if (wBest == -1e18) return false;
            if (wBest > bestSc) { bestSc = wBest; bwPos = pi; bId = wId; bD = wD; }
        }
        if (bwPos < 0) continue;
        const string &w = req[pending[bwPos]];
        int L = w.size(), cur = bId, nn = 0;
        if ((int)cells.size() < L) cells.resize(L);
        for (int k = 0; k < L; k++) { if (!grid[cur]) { grid[cur] = w[k]; cells[nn++] = cur; } cur = nxt(cur, bD); }
        if (!forb.empty())
            for (int t = 0; t < nn; t++) if (forbThrough(cells[t])) return false;
        pending.erase(pending.begin() + bwPos);
    }
    return fillBlanks();
}

// ---------------- LNS / ejection search at fixed N ----------------
// Keeps a conflict-free partial grid. A pending word goes to the placement with the lowest cost
// (ejection of the words in its way, minus overlap); ejected words become pending again.
vector<vector<int>> cover;      // words covering each cell
vector<int> posId, posD;        // placement of each word (-1 = pending)
vector<int> lastMove;           // iteration when the word was last placed (tabu)
vector<int> stampArr; int stampNow = 0;
vector<double> wgt;             // LNS ejection weight per word (grows each time it is ejected)

void placeWord(int w, int id, int d) {
    const string &s = req[w];
    int cur = id;
    for (size_t k = 0; k < s.size(); k++) { if (cover[cur].empty()) grid[cur] = s[k]; cover[cur].push_back(w); cur = nxt(cur, d); }
    posId[w] = id; posD[w] = d;
}
void removeWord(int w) {
    const string &s = req[w];
    int cur = posId[w], d = posD[w];
    for (size_t k = 0; k < s.size(); k++) {
        auto &cv = cover[cur];
        for (size_t t = 0; t < cv.size(); t++) if (cv[t] == w) { cv[t] = cv.back(); cv.pop_back(); break; }
        if (cv.empty()) grid[cur] = 0;
        cur = nxt(cur, d);
    }
    posId[w] = -1;
}

bool lns(double deadline) {
    int R = req.size();
    grid.assign(N * N, 0);
    cover.assign(N * N, {});
    posId.assign(R, -1); posD.assign(R, 0); lastMove.assign(R, -1000000);
    stampArr.assign(R, 0); stampNow = 0;
    vector<int> pending(R);
    iota(pending.begin(), pending.end(), 0);
    auto byLen = [&](int a, int b) { return req[a].size() < req[b].size(); };  // pop_back = longest
    vector<int> ej, bestEj, newCells;
    vector<unordered_set<int>> banned(R);
    int minPend = R;
    wgt.assign(R, 1.0);
    vector<double> ejCost(R, 0.0);
    for (long long it = 0; !pending.empty(); it++) {
        if ((it & 15) == 0 && elapsed() > deadline) {
#ifdef P2TUNE
            if (getenv("DBG")) cerr << "lns fail N=" << N << " it=" << it << " minPend=" << minPend << "\n";
#endif
            return false;
        }
        minPend = min<int>(minPend, pending.size());
        // choose next word: longest pending (with a little randomness)
        sort(pending.begin(), pending.end(), byLen);
        int pick = (int)pending.size() - 1;
        if (pending.size() > 1 && urand() < CFG2::LNS_RANDOM_PICK) pick = rnd() % pending.size();
        int w = pending[pick];
        pending.erase(pending.begin() + pick);
        const string &s = req[w];
        int L = s.size();
        for (int v = 0; v < R; v++)
            ejCost[v] = CFG2::EJECT_W * req[v].size() * wgt[v] + (it - lastMove[v] < CFG2::TABU ? CFG2::TABU_COST : 0.0);
        double bestC = 1e18; int bId = -1, bD = 0;
        for (int id = 0; id < N * N; id++) {
            const int r0 = id / N, c0 = id - (id / N) * N;
            for (int d = 0; d < 8; d++) {
                const int dr = DR[d], dc = DC[d];
                int r = r0, cc = c0, ov = 0; double c = 0;
                stampNow++;
                bool bad = false;
                for (int k = 0; k < L; k++) {
                    const int cur = r * N + cc;
                    char g = grid[cur];
                    if (g) {
                        if (g == s[k]) ov++;
                        else {
                            for (int v : cover[cur]) if (stampArr[v] != stampNow) {
                                stampArr[v] = stampNow;
                                c += ejCost[v];
                            }
                            if (c >= bestC + CFG2::OVERLAP_W * L) { bad = true; break; }
                        }
                    }
                    r += dr; if (r >= N) r -= N; else if (r < 0) r += N;
                    cc += dc; if (cc >= N) cc -= N; else if (cc < 0) cc += N;
                }
                if (bad) continue;
                c -= CFG2::OVERLAP_W * ov;
                c += CFG2::NOISE * urand();
                if (c < bestC && !(banned[w].size() && banned[w].count(id * 8 + d))) { bestC = c; bId = id; bD = d; }
            }
        }
        if (bId < 0) return false;
        // eject the words in the way
        {
            int cur = bId; ej.clear();
            stampNow++;
            for (int k = 0; k < L; k++) {
                if (grid[cur] && grid[cur] != s[k])
                    for (int v : cover[cur]) if (stampArr[v] != stampNow) { stampArr[v] = stampNow; ej.push_back(v); }
                cur = nxt(cur, bD);
            }
            for (int v : ej) { removeWord(v); pending.push_back(v); wgt[v] += CFG2::WEIGHT_INC; }
        }
        // place, then check forbidden words through newly lettered cells
        newCells.clear();
        { int cur = bId; for (int k = 0; k < L; k++) { if (cover[cur].empty()) newCells.push_back(cur); cur = nxt(cur, bD); } }
        placeWord(w, bId, bD);
        lastMove[w] = it;
        if (!forb.empty()) {
            bool badF = false;
            for (int c : newCells) if (forbThrough(c)) { badF = true; break; }
            if (badF) { removeWord(w); pending.push_back(w); banned[w].insert(bId * 8 + bD); }
        }
    }
    return fillBlanks();
}

// Guaranteed-valid layout: words on even rows (first-fit decreasing, one safe filler between
// words and before the wrap), all other cells safe filler letters. Any non-horizontal line
// alternates word rows and filler rows, so no forbidden word (which has no filler letter) fits.
bool rowFallback() {
    if (safeLetters.empty()) return false;
    int rows = N / 2;
    vector<int> used(rows, 0);
    vector<int> ord(req.size());
    iota(ord.begin(), ord.end(), 0);
    sort(ord.begin(), ord.end(), [&](int a, int b) { return req[a].size() > req[b].size(); });
    grid.assign(N * N, 0);
    for (int w : ord) {
        int L = req[w].size(), r = 0;
        while (r < rows && used[r] + L + 1 > N) r++;
        if (r == rows) return false;
        for (int k = 0; k < L; k++) grid[(2 * r) * N + used[r] + k] = req[w][k];
        used[r] += L + 1;
    }
    for (auto &g : grid) if (!g) g = 'A' + safeLetters[0];
    return true;
}
}  // namespace wsg

vector<vector<char>> generate_grid(vector<string> required_words, vector<string> forbidden_words) {
    using namespace wsg;
    T0 = chrono::steady_clock::now();
    timeUp = false;
#ifdef P2TUNE
    auto env = [](const char *k, double &v) { if (const char *e = getenv(k)) v = atof(e); };
    auto envi = [](const char *k, int &v) { if (const char *e = getenv(k)) v = atoi(e); };
    env("NZ", CFG2::NOISE); env("OW", CFG2::OVERLAP_W); env("CW", CFG2::CONTACT_W); env("EW", CFG2::EJECT_W);
    env("TB", CFG2::TABU); env("WI", CFG2::WEIGHT_INC); env("RP", CFG2::LNS_RANDOM_PICK); env("SL", CFG2::LNS_SLICE_S);
    envi("GR", CFG2::GREEDY_PER_ROUND); envi("CK", CFG2::CANDS_K); env("ON", CFG2::ORDER_NOISE);
#endif
    req.clear(); forb.clear();
    auto clean = [](const string &s) { string t; for (char ch : s) if (isalpha((unsigned char)ch)) t += (char)toupper(ch); return t; };
    ll S = 0; int Lmax = 1;
    for (auto &w : required_words) { string t = clean(w); S += t.size(); Lmax = max<int>(Lmax, t.size()); if (!t.empty()) req.push_back(t); }
    for (auto &w : forbidden_words) { string t = clean(w); if (!t.empty()) forb.push_back(t); }
    // dedupe; drop words contained (forwards or backwards) in a longer required word
    sort(req.begin(), req.end()); req.erase(unique(req.begin(), req.end()), req.end());
    {
        vector<string> keep;
        for (size_t i = 0; i < req.size(); i++) {
            bool sub = false;
            string rv(req[i].rbegin(), req[i].rend());
            for (size_t j = 0; j < req.size() && !sub; j++)
                if (i != j && req[j].size() > req[i].size() && (req[j].find(req[i]) != string::npos || req[j].find(rv) != string::npos)) sub = true;
            if (!sub) keep.push_back(req[i]);
        }
        req = keep;
    }
    {
        bool used[26] = {};
        for (auto &f : forb) for (char ch : f) used[ch - 'A'] = true;
        safeLetters.clear();
        for (int i = 0; i < 26; i++) if (!used[i]) safeLetters.push_back(i);
    }
    int Nlo = max<int>(Lmax, (int)ceil(sqrt(S / 1.7) - 1e-9));
    vector<char> bestGrid; int bestN = INT_MAX;
    // 0) guaranteed-valid fallback (needs a letter absent from every forbidden word)
    if (!safeLetters.empty()) {
        for (N = max<int>(Lmax + 1, (int)sqrt(2.0 * S)); ; N++) if (rowFallback()) { bestGrid = grid; bestN = N; break; }
    }
    // 1) search upward for a feasible N, starting near density 1
    int Nstart = max<int>(Nlo, (int)ceil(sqrt((double)S)));
    for (N = Nstart; N < bestN && elapsed() < CFG2::TIME_BUDGET_S; N += max(1, N / 25)) {
        buildTables();
        bool ok = false;
        for (int t = 0; t < CFG2::FIRST_TRIES && !ok && !timeUp; t++) ok = attempt();
        if (!ok) ok = lns(min(CFG2::TIME_BUDGET_S, elapsed() + CFG2::FIRST_LNS_S));
        if (ok) { bestGrid = grid; bestN = N; break; }
    }
    // 2) shrink N while attempts succeed
    if (bestN != INT_MAX) {
        for (N = bestN - 1; N >= Nlo && elapsed() < CFG2::TIME_BUDGET_S; ) {
            buildTables();
            bool ok = false;
            while (!ok && elapsed() < CFG2::TIME_BUDGET_S) {
                for (int t = 0; t < CFG2::GREEDY_PER_ROUND && !ok; t++) ok = attempt();
                if (!ok) ok = lns(min(CFG2::TIME_BUDGET_S, elapsed() + CFG2::LNS_SLICE_S));
            }
            if (!ok) break;
            bestGrid = grid; bestN = N; N--;
        }
    }
    // 3) fallback: one word per row (wrapping makes rows independent only if fillers are safe)
    if (bestN == INT_MAX) {
        N = max<int>(Lmax, (int)req.size());
        buildTables();
        grid.assign(N * N, 0);
        for (size_t i = 0; i < req.size(); i++) for (size_t k = 0; k < req[i].size(); k++) grid[i * N + k] = req[i][k];
        if (!fillBlanks()) for (auto &g : grid) if (!g) g = 'A';
        bestGrid = grid; bestN = N;
    }
    N = bestN;
    vector<vector<char>> res(N, vector<char>(N));
    for (int r = 0; r < N; r++) for (int c = 0; c < N; c++) res[r][c] = bestGrid[r * N + c];
    return res;
}
