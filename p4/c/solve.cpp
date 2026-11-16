// p4-c: Masyu shortest loop by simulated annealing over the loop's INTERIOR REGION.
// Any simple loop through cell centres is the boundary of a simply connected set of "dual squares"
// (square (i,j) has corners at cells (i,j),(i,j+1),(i+1,j),(i+1,j+1)) with no diagonal pinch.
// Move = flip one square, allowed only if it keeps the region simply connected (Yokoi number = 1)
// and creates no 2x2 diagonal pattern. Cost = loop length + LAMBDA * violated pearls.
#include <bits/stdc++.h>
using namespace std;

namespace CFG4 {
double TIME_BUDGET_S = 0.85;     // total SA time (first second is free)
double LAMBDA = 6.0;             // penalty per violated pearl
double T_START = 2.0;            // SA start temperature
double T_END = 0.15;             // SA end temperature
int RESTARTS = 4;                // independent SA runs (time split evenly)
int INIT_MARGIN = 1;             // initial rectangle = pearl bounding box grown by this many cells
}

namespace msy {
int R, C, SR, SC;                  // cells R x C, squares SR x SC = (R-1) x (C-1)
vector<string> G;
vector<unsigned char> sq;          // padded (SR+2) x (SC+2), 1 = inside
inline unsigned char &S(int i, int j) { return sq[(i + 1) * (SC + 2) + (j + 1)]; }
inline int sqv(int i, int j) { return sq[(i + 1) * (SC + 2) + (j + 1)]; }
uint64_t rs = 0x2545F4914F6CDD1DULL;
inline uint64_t rnd() { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return rs; }
inline double urand() { return (rnd() >> 11) * (1.0 / 9007199254740992.0); }
chrono::steady_clock::time_point T0;
double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }

enum { DN = 1, DE = 2, DS = 4, DW = 8 };
inline int maskAt(int r, int c) {
    if (r < 0 || r >= R || c < 0 || c >= C) return 0;
    int nw = sqv(r - 1, c - 1), ne = sqv(r - 1, c), sw = sqv(r, c - 1), se = sqv(r, c);
    int m = 0;
    if (nw != ne) m |= DN;
    if (ne != se) m |= DE;
    if (sw != se) m |= DS;
    if (nw != sw) m |= DW;
    return m;
}
inline bool straight(int m) { return m == (DN | DS) || m == (DE | DW); }
inline int viol(int r, int c) {
    char p = G[r][c];
    if (p == '.') return 0;
    int m = maskAt(r, c);
    if (!m) return 1;
    // neighbours along the loop
    int a[2], b[2], k = 0;
    if (m & DN) { a[k] = r - 1; b[k++] = c; }
    if (m & DE) { a[k] = r; b[k++] = c + 1; }
    if (m & DS) { a[k] = r + 1; b[k++] = c; }
    if (m & DW) { a[k] = r; b[k++] = c - 1; }
    if (k != 2) return 1;
    bool s0 = straight(maskAt(a[0], b[0])), s1 = straight(maskAt(a[1], b[1]));
    if (p == 'W') return (!straight(m) || (s0 && s1)) ? 1 : 0;
    return (straight(m) || !s0 || !s1) ? 1 : 0;   // 'B'
}
// Yokoi 4-connectivity number of square (i,j)'s 8-ring (region excluding the square itself)
inline int yokoi(int i, int j) {
    int x[9];
    x[1] = sqv(i, j + 1); x[2] = sqv(i - 1, j + 1); x[3] = sqv(i - 1, j); x[4] = sqv(i - 1, j - 1);
    x[5] = sqv(i, j - 1); x[6] = sqv(i + 1, j - 1); x[7] = sqv(i + 1, j); x[8] = sqv(i + 1, j + 1);
    int s = 0;
    for (int k = 1; k <= 7; k += 2) { int k1 = k + 1, k2 = (k + 2 > 8) ? 1 : k + 2; s += x[k] - x[k] * x[k1] * x[k2]; }
    return s;
}
// would square (i,j), set to v, create a diagonal 2x2 pattern?
inline bool diagBad(int i, int j) {
    for (int di = -1; di <= 0; di++) for (int dj = -1; dj <= 0; dj++) {
        int a = sqv(i + di, j + dj), b = sqv(i + di, j + dj + 1), c = sqv(i + di + 1, j + dj), d = sqv(i + di + 1, j + dj + 1);
        if (a == d && b == c && a != b) return true;
    }
    return false;
}

vector<pair<int, int>> pearls;
vector<vector<int>> pearlNear;   // per square: indices of pearls whose rule may change

int loopLen() { int L = 0; for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) L += maskAt(r, c) != 0; return L; }
int totalViol() { int V = 0; for (auto &p : pearls) V += viol(p.first, p.second); return V; }

vector<string> render(const vector<unsigned char> &state) {
    auto save = sq; sq = state;
    vector<string> out(R, string(C, '.'));
    for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) {
        int m = maskAt(r, c); char ch = '.';
        if (m == (DN | DS)) ch = '|'; else if (m == (DE | DW)) ch = '-';
        else if (m == (DN | DE)) ch = 'L'; else if (m == (DN | DW)) ch = 'J';
        else if (m == (DS | DE)) ch = 'F'; else if (m == (DS | DW)) ch = '7';
        out[r][c] = ch;
    }
    sq = save;
    return out;
}
}  // namespace msy

vector<string> solve(vector<string> grid) {
    using namespace msy;
    T0 = chrono::steady_clock::now();
#ifdef P4DBG
    { auto env = [](const char *k, double &v) { if (const char *e = getenv(k)) v = atof(e); };
      env("LA", CFG4::LAMBDA); env("TS", CFG4::T_START); env("TE", CFG4::T_END); env("TB", CFG4::TIME_BUDGET_S);
      if (const char *e = getenv("RS")) CFG4::RESTARTS = atoi(e); if (const char *e = getenv("IM")) CFG4::INIT_MARGIN = atoi(e); }
#endif
    G.clear();
    for (auto &row : grid) { string t; for (char ch : row) if (ch == '.' || ch == 'W' || ch == 'B') t += ch; if (!t.empty()) G.push_back(t); }
    R = G.size(); C = R ? G[0].size() : 0;
    SR = R - 1; SC = C - 1;
    pearls.clear();
    int r0 = R, r1 = -1, c0 = C, c1 = -1;
    for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) if (G[r][c] != '.') {
        pearls.push_back({r, c}); r0 = min(r0, r); r1 = max(r1, r); c0 = min(c0, c); c1 = max(c1, c);
    }
    pearlNear.assign(SR * SC, {});
    {
        vector<int> pid(R * C, -1);
        for (size_t k = 0; k < pearls.size(); k++) pid[pearls[k].first * C + pearls[k].second] = k;
        for (int i = 0; i < SR; i++) for (int j = 0; j < SC; j++)
            for (int r = i - 1; r <= i + 2; r++) for (int c = j - 1; c <= j + 2; c++)
                if (r >= 0 && r < R && c >= 0 && c < C && pid[r * C + c] >= 0) pearlNear[i * SC + j].push_back(pid[r * C + c]);
    }
    vector<unsigned char> best; int bestLen = INT_MAX;
    int runs = max(1, CFG4::RESTARTS);
    for (int run = 0; run < runs; run++) {
        double tEnd = CFG4::TIME_BUDGET_S * (run + 1) / runs;
        double tBeg = elapsed();
        // initial region: rectangle of squares around the pearl bounding box
        sq.assign((SR + 2) * (SC + 2), 0);
        int a0 = max(0, r0 - CFG4::INIT_MARGIN), a1 = min(SR - 1, max(a0, r1 + CFG4::INIT_MARGIN - 1));
        int b0 = max(0, c0 - CFG4::INIT_MARGIN), b1 = min(SC - 1, max(b0, c1 + CFG4::INIT_MARGIN - 1));
        for (int i = a0; i <= a1; i++) for (int j = b0; j <= b1; j++) S(i, j) = 1;
        int len = loopLen(), V = totalViol();
        if (V == 0 && len < bestLen) { bestLen = len; best = sq; }
        double T = CFG4::T_START;
        long long it = 0;
        while (true) {
            if ((it & 1023) == 0) {
                double t = elapsed();
                if (t > tEnd) break;
                double frac = (t - tBeg) / max(1e-9, tEnd - tBeg);
                T = CFG4::T_START * pow(CFG4::T_END / CFG4::T_START, frac);
            }
            it++;
            int i = rnd() % SR, j = rnd() % SC;
            // must touch the boundary: some 4-neighbour differs
            int v = sqv(i, j);
            if (sqv(i - 1, j) == v && sqv(i + 1, j) == v && sqv(i, j - 1) == v && sqv(i, j + 1) == v) continue;
            if (yokoi(i, j) != 1) continue;
            // delta
            int dl = 0, dv = 0;
            for (int r = i; r <= i + 1; r++) for (int c = j; c <= j + 1; c++) dl -= maskAt(r, c) != 0;
            auto &pn = pearlNear[i * SC + j];
            for (int p : pn) dv -= viol(pearls[p].first, pearls[p].second);
            S(i, j) = v ^ 1;
            if (diagBad(i, j)) { S(i, j) = v; continue; }
            for (int r = i; r <= i + 1; r++) for (int c = j; c <= j + 1; c++) dl += maskAt(r, c) != 0;
            for (int p : pn) dv += viol(pearls[p].first, pearls[p].second);
            double d = dl + CFG4::LAMBDA * dv;
            if (d <= 0 || urand() < exp(-d / T)) {
                len += dl; V += dv;
                if (V == 0 && len < bestLen) { bestLen = len; best = sq; }
            } else S(i, j) = v;
        }
#ifdef P4DBG
        if (getenv("VERB")) cerr << "run " << run << " it=" << it << " len=" << len << " V=" << V << " best=" << bestLen << "\n";
#endif
    }
    if (best.empty()) {
        // no valid loop found: return the last state (invalid) rather than nothing
        return render(sq);
    }
    return render(best);
}
