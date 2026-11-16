// p2-b: word search generation on a torus.
// For a fixed side N, simulated annealing over word placements (row, col, direction);
// cost = sum over cells of (#words covering the cell - largest same-letter count).
// Cost 0 = consistent grid. Free cells get a letter that occurs in no forbidden word, then the
// grid is checked for forbidden words (trie walk, wrapping). Shrink N while time remains.
#include <bits/stdc++.h>
using namespace std;

namespace P2CFG {
double TIME_BUDGET_S = 1.5;      // total time inside generate_grid
double FIRST_SLICE = 0.15;       // share of the budget for each attempt while no grid is found yet
double T_START = 0.3;            // SA start temperature (cost units = conflicting letters)
double T_END = 0.1;              // SA end temperature
long long ITERS_PER_TEMP_CYCLE = 20000;   // SA iterations in the first temperature cycle
double CYCLE_GROWTH = 2.0;       // each reheat cycle is this much longer than the previous
double OCC_W = 0.2;              // energy per occupied cell (rewards shared letters); conflicts weigh 1
double P_SHIFT = 0.0;            // probability of shifting the word by one cell
double P_ROT = 0.0;              // probability of turning the word around one of its letters
double P_BEST = 0.05;            // probability of moving the word to its best placement (min-conflicts)
int CONF_TRIES = 3;              // re-pick a word up to this many times until it touches a conflict
double SCAN_FRAC = 0.3;          // greedy-only scan for the smallest N may use this share of the budget
int SCAN_RESTARTS = 3;           // greedy restarts per N in the scan
int WARM_START = 0;              // 1 = start N-1 from the N solution's placements (mod N-1)
double P_CROSS = 0.7;            // probability a move crosses the word with a placed word
int REPAIR_ROUNDS = 200;         // free-cell repair rounds when no safe filler letter exists
}

namespace p2b {
const int DR[8] = {0, 1, 1, 1, 0, -1, -1, -1};
const int DC[8] = {1, 1, 0, -1, -1, -1, 0, 1};

chrono::steady_clock::time_point T0;
double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }
uint64_t rs;
inline uint64_t rnd() { rs ^= rs << 7; rs ^= rs >> 9; return rs; }
inline int rndInt(int k) { return (int)(rnd() % (uint64_t)k); }
inline double urand() { return (rnd() >> 11) * (1.0 / 9007199254740992.0); }

// ---------- forbidden-word trie ----------
struct Trie {
    vector<array<int, 26>> nx;
    vector<char> term;
    int maxDepth = 0;
    void init() { nx.assign(1, {}); nx[0].fill(-1); term.assign(1, 0); maxDepth = 0; }
    void add(const string &w) {
        int u = 0;
        for (char ch : w) {
            int c = ch - 'A';
            if (nx[u][c] < 0) { nx[u][c] = nx.size(); nx.push_back({}); nx.back().fill(-1); term.push_back(0); }
            u = nx[u][c];
        }
        term[u] = 1;
        maxDepth = max(maxDepth, (int)w.size());
    }
};
Trie trie;

// any forbidden word in grid g (side N)? if cellsOut, collect cells of the first hit
bool hasForbidden(const vector<char> &g, int N, vector<int> *cellsOut) {
    if (trie.maxDepth == 0) return false;
    for (int r = 0; r < N; r++) for (int c = 0; c < N; c++) for (int d = 0; d < 8; d++) {
        int u = 0, rr = r, cc = c;
        for (int k = 0; k < trie.maxDepth; k++) {
            u = trie.nx[u][g[rr * N + cc] - 'A'];
            if (u < 0) break;
            if (trie.term[u]) {
                if (cellsOut) {
                    cellsOut->clear();
                    int r2 = r, c2 = c;
                    for (int t = 0; t <= k; t++) { cellsOut->push_back(r2 * N + c2); r2 = (r2 + DR[d] + N) % N; c2 = (c2 + DC[d] + N) % N; }
                }
                return true;
            }
            rr = (rr + DR[d] + N) % N; cc = (cc + DC[d] + N) % N;
        }
    }
    return false;
}

bool appears(const vector<char> &g, int N, const string &w) {
    if ((int)w.size() > N) return false;
    for (int r = 0; r < N; r++) for (int c = 0; c < N; c++) {
        if (g[r * N + c] != w[0]) continue;
        for (int d = 0; d < 8; d++) {
            int rr = r, cc = c, k = 0;
            for (; k < (int)w.size(); k++) {
                if (g[rr * N + cc] != w[k]) break;
                rr = (rr + DR[d] + N) % N; cc = (cc + DC[d] + N) % N;
            }
            if (k == (int)w.size()) return true;
        }
    }
    return false;
}

// ---------- SA over placements ----------
vector<string> words;            // words to place (deduped, substrings removed)
vector<array<vector<int>, 26>> pos;  // per word, positions of each letter
int fillLetter;                  // -1 if every letter is in some forbidden word

struct Solver {
    int N, Wn;
    vector<array<int, 26>> cnt;  // per cell letter counts
    vector<int> tot, conf;
    vector<int> pr, pc, pd;      // placement per word
    long long cost;              // total conflicts
    long long minCost = LLONG_MAX, iters = 0;
    double energy;               // conflicts + OCC_W * occupied cells

    inline int cellOf(int i, int k) const {
        int r = ((pr[i] + DR[pd[i]] * k) % N + N) % N;
        int c = ((pc[i] + DC[pd[i]] * k) % N + N) % N;
        return r * N + c;
    }
    inline int confOf(int cell) const {
        int mx = 0;
        const auto &a = cnt[cell];
        for (int x = 0; x < 26; x++) mx = max(mx, a[x]);
        return tot[cell] - mx;
    }
    // add (sign=+1) or remove (-1) word i at its placement; returns cost delta
    double apply(int i, int sign) {
        long long d = 0, occ = 0;
        const string &w = words[i];
        for (int k = 0; k < (int)w.size(); k++) {
            int cell = cellOf(i, k);
            cnt[cell][w[k] - 'A'] += sign;
            tot[cell] += sign;
            if (sign > 0 && tot[cell] == 1) occ++;
            if (sign < 0 && tot[cell] == 0) occ--;
            int nc = confOf(cell);
            d += nc - conf[cell];
            conf[cell] = nc;
        }
        cost += d;
        double de = d + P2CFG::OCC_W * occ;
        energy += de;
        return de;
    }
    bool touchesConflict(int i) const {
        for (int k = 0; k < (int)words[i].size(); k++) if (conf[cellOf(i, k)]) return true;
        return false;
    }
    void randomPlace(int i) { pr[i] = rndInt(N); pc[i] = rndInt(N); pd[i] = rndInt(8); }
    // place word i crossing a random placed word j at a shared letter (fallback random)
    void crossPlace(int i) {
        for (int tries = 0; tries < 4; tries++) {
            int j = rndInt(Wn);
            if (j == i) continue;
            int t = rndInt(words[j].size());
            int ch = words[j][t] - 'A';
            const auto &ps = pos[i][ch];
            if (ps.empty()) continue;
            int k = ps[rndInt(ps.size())];
            int cell = cellOf(j, t);
            int d = rndInt(8);
            int r = cell / N, c = cell % N;
            pr[i] = ((r - DR[d] * k) % N + N) % N;
            pc[i] = ((c - DC[d] * k) % N + N) % N;
            pd[i] = d;
            return;
        }
        randomPlace(i);
    }

    // start from given placements, coordinates taken mod n
    void initFrom(int n, vector<int> r0, vector<int> c0, vector<int> d0) {
        N = n; Wn = words.size();
        cnt.assign(N * N, {});
        for (auto &a : cnt) a.fill(0);
        tot.assign(N * N, 0); conf.assign(N * N, 0);
        pr.resize(Wn); pc.resize(Wn); pd.resize(Wn);
        cost = 0; energy = 0;
        for (int i = 0; i < Wn; i++) { pr[i] = r0[i] % N; pc[i] = c0[i] % N; pd[i] = d0[i]; apply(i, +1); }
    }
    // best placement for word i (not applied): least conflicts + OCC_W * new cells, random ties
    void bestPlace(int i) {
        const string &w = words[i];
        double bestD = 1e18; int br = 0, bc = 0, bd = 0;
        for (int r = 0; r < N; r++) for (int c = 0; c < N; c++) for (int d = 0; d < 8; d++) {
            double v = 0;
            int rr = r, cc = c;
            for (int k = 0; k < (int)w.size() && v < bestD; k++) {
                int cell = rr * N + cc;
                if (tot[cell] == 0) v += P2CFG::OCC_W;
                else if (cnt[cell][w[k] - 'A'] != tot[cell]) v += 1;
                rr += DR[d]; cc += DC[d];
                if (rr < 0) rr += N; else if (rr >= N) rr -= N;
                if (cc < 0) cc += N; else if (cc >= N) cc -= N;
            }
            v += urand() * 1e-3;
            if (v < bestD) { bestD = v; br = r; bc = c; bd = d; }
        }
        pr[i] = br; pc[i] = bc; pd[i] = bd;
    }
    void init(int n) {
        N = n; Wn = words.size();
        cnt.assign(N * N, {});
        for (auto &a : cnt) a.fill(0);
        tot.assign(N * N, 0); conf.assign(N * N, 0);
        pr.assign(Wn, 0); pc.assign(Wn, 0); pd.assign(Wn, 0);
        cost = 0; energy = 0;
        // greedy initial: each word (longest first) at the placement with the least
        // conflicts + OCC_W * new cells, over ALL N*N*8 placements (ties broken randomly)
        vector<int> order(Wn);
        iota(order.begin(), order.end(), 0);
        for (int a = Wn - 1; a > 0; a--) swap(order[a], order[rndInt(a + 1)]);
        stable_sort(order.begin(), order.end(), [](int a, int b) { return words[a].size() > words[b].size(); });
        for (int idx = 0; idx < Wn; idx++) {
            int i = order[idx];
            const string &w = words[i];
            double bestD = 1e18; int br = 0, bc = 0, bd = 0;
            for (int r = 0; r < N; r++) for (int c = 0; c < N; c++) for (int d = 0; d < 8; d++) {
                double v = 0;
                int rr = r, cc = c;
                for (int k = 0; k < (int)w.size() && v < bestD; k++) {
                    int cell = rr * N + cc;
                    if (tot[cell] == 0) v += P2CFG::OCC_W;
                    else if (cnt[cell][w[k] - 'A'] != tot[cell]) v += 1;
                    rr += DR[d]; cc += DC[d];
                    if (rr < 0) rr += N; else if (rr >= N) rr -= N;
                    if (cc < 0) cc += N; else if (cc >= N) cc -= N;
                }
                v += urand() * 1e-3;
                if (v < bestD) { bestD = v; br = r; bc = c; bd = d; }
            }
            pr[i] = br; pc[i] = bc; pd[i] = bd;
            apply(i, +1);
        }
    }

    // run SA until cost 0 or deadline; returns true on cost 0
    bool anneal(double deadline) {
        if (cost == 0) return true;
        long long it = 0, cycStart = 0;
        double T = P2CFG::T_START;
        double cyc = P2CFG::ITERS_PER_TEMP_CYCLE;
        while (true) {
            if ((it & 255) == 0) {
                if (elapsed() > deadline) return false;
                if (it - cycStart >= (long long)cyc) { cycStart = it; cyc *= P2CFG::CYCLE_GROWTH; }
                double f = (double)(it - cycStart) / cyc;
                T = P2CFG::T_START * pow(P2CFG::T_END / P2CFG::T_START, f);
            }
            it++; iters++;
            int i = rndInt(Wn);
            for (int t = 0; t < P2CFG::CONF_TRIES && !touchesConflict(i); t++) i = rndInt(Wn);
            int or_ = pr[i], oc = pc[i], od = pd[i];
            double d = apply(i, -1);
            double u = urand();
            if (u < P2CFG::P_SHIFT) {
                int dd = rndInt(8);
                pr[i] = (pr[i] + DR[dd] + N) % N; pc[i] = (pc[i] + DC[dd] + N) % N;
            } else if (u < P2CFG::P_SHIFT + P2CFG::P_ROT) {
                int k = rndInt(words[i].size());
                int cell = cellOf(i, k), r = cell / N, c = cell % N;
                int d2 = rndInt(8);
                pr[i] = ((r - DR[d2] * k) % N + N) % N; pc[i] = ((c - DC[d2] * k) % N + N) % N; pd[i] = d2;
            } else if (u < P2CFG::P_SHIFT + P2CFG::P_ROT + P2CFG::P_BEST) {
                bestPlace(i);
            } else if (urand() < P2CFG::P_CROSS) crossPlace(i); else randomPlace(i);
            d += apply(i, +1);
            if (d <= 0 || urand() < exp(-d / T)) {
                if (cost < minCost) minCost = cost;
                if (cost == 0) return true;
            } else {
                apply(i, -1);
                pr[i] = or_; pc[i] = oc; pd[i] = od;
                apply(i, +1);
            }
        }
    }

    // build grid from a cost-0 placement; free cells = filler; false if forbidden words remain
    bool build(vector<char> &g) {
        g.assign(N * N, 0);
        vector<char> fixedCell(N * N, 0);
        for (int cell = 0; cell < N * N; cell++) {
            if (tot[cell] == 0) continue;
            fixedCell[cell] = 1;
            for (int x = 0; x < 26; x++) if (cnt[cell][x]) { g[cell] = 'A' + x; break; }
        }
        for (int cell = 0; cell < N * N; cell++)
            if (!fixedCell[cell]) g[cell] = fillLetter >= 0 ? 'A' + fillLetter : 'A' + rndInt(26);
        vector<int> hit;
        for (int round = 0; round < P2CFG::REPAIR_ROUNDS; round++) {
            if (!hasForbidden(g, N, &hit)) return true;
            vector<int> freeHit;
            for (int cell : hit) if (!fixedCell[cell]) freeHit.push_back(cell);
            if (freeHit.empty()) return false;
            g[freeHit[rndInt(freeHit.size())]] = 'A' + rndInt(26);
        }
        return !hasForbidden(g, N, nullptr);
    }
};

string clean(const string &s) {
    string t;
    for (char ch : s) {
        if (ch >= 'a' && ch <= 'z') ch = ch - 'a' + 'A';
        if (ch >= 'A' && ch <= 'Z') t += ch;
    }
    return t;
}

vector<vector<char>> toRows(const vector<char> &g, int N) {
    vector<vector<char>> res(N, vector<char>(N));
    for (int r = 0; r < N; r++) for (int c = 0; c < N; c++) res[r][c] = g[r * N + c];
    return res;
}

vector<vector<char>> solve(const vector<string> &reqIn, const vector<string> &forbIn) {
    T0 = chrono::steady_clock::now();
    rs = 88172645463325252ULL;
    vector<string> req, forb;
    for (auto &w : reqIn) { string t = clean(w); if (!t.empty()) req.push_back(t); }
    for (auto &w : forbIn) { string t = clean(w); if (!t.empty()) forb.push_back(t); }
    int maxLen = 1;
    long long S = 0;
    for (auto &w : req) { maxLen = max(maxLen, (int)w.size()); S += w.size(); }
    trie.init();
    for (auto &w : forb) trie.add(w);
    // filler letter: absent from every forbidden word; prefer one absent from required words too
    {
        array<int, 26> inF{}, inR{};
        for (auto &w : forb) for (char ch : w) inF[ch - 'A'] = 1;
        for (auto &w : req) for (char ch : w) inR[ch - 'A']++;
        fillLetter = -1;
        int bestR = INT_MAX;
        for (int x = 0; x < 26; x++) if (!inF[x] && inR[x] < bestR) { bestR = inR[x]; fillLetter = x; }
    }
    // words to place: dedupe, drop substrings of other words (either reading direction)
    {
        vector<string> u = req;
        sort(u.begin(), u.end(), [](const string &a, const string &b) { return a.size() != b.size() ? a.size() > b.size() : a < b; });
        u.erase(unique(u.begin(), u.end()), u.end());
        words.clear();
        for (auto &w : u) {
            string rw(w.rbegin(), w.rend());
            bool sub = false;
            for (auto &v : words) if (v.find(w) != string::npos || v.find(rw) != string::npos) { sub = true; break; }
            if (!sub) words.push_back(w);
        }
    }
    pos.assign(words.size(), {});
    for (size_t i = 0; i < words.size(); i++)
        for (int k = 0; k < (int)words[i].size(); k++) pos[i][words[i][k] - 'A'].push_back(k);

    // Fallback grid: words on even rows (filler between rows), always valid unless impossible.
    int Wn = words.size();
    if (Wn == 0) return toRows(vector<char>(1, 'A' + max(0, fillLetter)), 1);
    int bestN = max(maxLen + 1, 2 * Wn);
    vector<char> bestG(bestN * bestN, fillLetter >= 0 ? 'A' + fillLetter : 'Q');
    for (int i = 0; i < Wn; i++) for (int k = 0; k < (int)words[i].size(); k++) bestG[(2 * i) * bestN + k] = words[i][k];

    long long Su = 0;
    for (auto &w : words) Su += w.size();
    int fullN = (int)floor(sqrt((double)S / 1.7));   // any N <= fullN already scores 100
    int N = max(maxLen, (int)ceil(sqrt((double)Su)));
    int Nlow = max(maxLen, fullN);
    bool found = false;
    Solver sv;
    vector<char> g;
    // greedy-only scan upwards from the lower bound: first N where a greedy grid is valid
    for (int n2 = Nlow; n2 < bestN && elapsed() < P2CFG::SCAN_FRAC * P2CFG::TIME_BUDGET_S; n2++) {
        bool ok = false;
        for (int rep = 0; rep < P2CFG::SCAN_RESTARTS && !ok; rep++) {
            sv.init(n2);
            if (sv.cost == 0 && sv.build(g)) ok = true;
        }
#ifdef LOCAL
        if (getenv("DBG")) cerr << "scan N=" << n2 << " ok=" << ok << " cost=" << sv.cost << " t=" << elapsed() << "\n";
#endif
        if (ok) { bestN = n2; bestG = g; found = true; N = n2 - 1; break; }
    }
    while (N >= Nlow && N < bestN && elapsed() < P2CFG::TIME_BUDGET_S) {
        // until the first grid is found, give each N a slice; afterwards all remaining time
        double deadline = found ? P2CFG::TIME_BUDGET_S
                                : min(P2CFG::TIME_BUDGET_S, elapsed() + P2CFG::FIRST_SLICE * P2CFG::TIME_BUDGET_S);
        sv.minCost = LLONG_MAX; sv.iters = 0;
        if (found && P2CFG::WARM_START) sv.initFrom(N, sv.pr, sv.pc, sv.pd);
        else sv.init(N);
        bool ok = false;
        while (elapsed() < deadline) {
            if (!sv.anneal(deadline)) break;
            if (sv.build(g)) { ok = true; break; }
            // forbidden word formed: kick a few words and continue
            for (int t = 0; t < 3; t++) { int i = rndInt(Wn); sv.apply(i, -1); sv.randomPlace(i); sv.apply(i, +1); }
        }
#ifdef LOCAL
        if (getenv("DBG")) cerr << "N=" << N << " ok=" << ok << " minCost=" << sv.minCost << " iters=" << sv.iters << " t=" << elapsed() << "\n";
#endif
        if (ok) {
            bestN = N; bestG = g; found = true;
            if (N <= max(maxLen, fullN)) break;
            N--;
        } else if (!found) {
            N++;
        } else break;
    }
    // final safety check: all required words present, no forbidden word
    bool valid = true;
    for (auto &w : req) if (!appears(bestG, bestN, w)) { valid = false; break; }
    if (hasForbidden(bestG, bestN, nullptr)) valid = false;
#ifdef LOCAL
    cerr << "N " << bestN << " S " << S << " Su " << Su << " words " << Wn << " maxLen " << maxLen
         << " fullN " << fullN << " density " << (double)S / bestN / bestN << " valid " << valid
         << " t " << elapsed() << "\n";
#endif
    return toRows(bestG, bestN);
}
}  // namespace p2b

vector<vector<char>> generate_grid(vector<string> required_words, vector<string> forbidden_words) {
    return p2b::solve(required_words, forbidden_words);
}

#ifdef LOCAL
int main() {
    if (getenv("TB")) P2CFG::TIME_BUDGET_S = atof(getenv("TB"));
    if (getenv("TS")) P2CFG::T_START = atof(getenv("TS"));
    if (getenv("TE")) P2CFG::T_END = atof(getenv("TE"));
    if (getenv("CYC")) P2CFG::ITERS_PER_TEMP_CYCLE = atoll(getenv("CYC"));
    if (getenv("CG")) P2CFG::CYCLE_GROWTH = atof(getenv("CG"));
    if (getenv("WS")) P2CFG::WARM_START = atoi(getenv("WS"));
    if (getenv("OW")) P2CFG::OCC_W = atof(getenv("OW"));
    if (getenv("CT")) P2CFG::CONF_TRIES = atoi(getenv("CT"));
    if (getenv("PS")) P2CFG::P_SHIFT = atof(getenv("PS"));
    if (getenv("PR")) P2CFG::P_ROT = atof(getenv("PR"));
    if (getenv("SF")) P2CFG::SCAN_FRAC = atof(getenv("SF"));
    if (getenv("SR")) P2CFG::SCAN_RESTARTS = atoi(getenv("SR"));
    if (getenv("PB")) P2CFG::P_BEST = atof(getenv("PB"));
    if (getenv("PC")) P2CFG::P_CROSS = atof(getenv("PC"));
    int R, F;
    vector<string> req, forb;
    if (!(cin >> R)) return 0;
    req.resize(R);
    for (auto &w : req) cin >> w;
    if (!(cin >> F)) F = 0;
    forb.resize(F);
    for (auto &w : forb) cin >> w;
    auto g = generate_grid(req, forb);
    string out;
    for (auto &row : g) {
        for (size_t j = 0; j < row.size(); j++) { out += row[j]; if (j + 1 < row.size()) out += ' '; }
        out += '\n';
    }
    cout << out;
}
#endif
