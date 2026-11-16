// p4-b: Masyu by edge constraint propagation + DFS, then randomised restarts for shorter loops.
// Edge states: -1 unknown, 0 off, 1 on. Rules: cell degree 0 or 2 (pearls 2), black/white pearl
// rules, and no sub-loop: chains keep partner endpoints; closing a chain is only allowed when it
// holds every on-edge and every pearl. Branch at the chain end with the fewest unknown edges.
#include <bits/stdc++.h>
using namespace std;

namespace P4CFG {
double FIRST_DEADLINE_S = 8.0;   // keep searching for a first valid loop up to this time
double IMPROVE_UNTIL_S = 0.9;    // after a loop is found, try restarts for shorter loops until here
long long RESTART_NODES = 20000; // node limit of one randomised restart (improvement phase)
double HARD_STOP_S = 8.5;        // DFS aborts at this time whatever happens
double ORDER_NOISE = 3.0;        // random noise (cells) added to the distance ordering on restarts
int FOCUS = 1;                   // branch on the longest chain's end (1) or the most constrained end (0)
int BB = 1;                      // 1 = prune restarts with on-edges >= best length (branch and bound)
}

namespace p4b {
const int DR[4] = {-1, 0, 1, 0};  // N E S W
const int DC[4] = {0, 1, 0, -1};

chrono::steady_clock::time_point T0;
double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }
uint64_t rs;
inline uint64_t rnd() { rs ^= rs << 7; rs ^= rs >> 9; return rs; }

int R, C, NC, NE, OFFE;          // cells, edges, sentinel off edge
vector<char> pearl;              // 0, 'W', 'B'
vector<array<int, 4>> ce;        // cell -> edge per dir (OFFE if outside)
vector<array<int, 4>> nb;        // cell -> neighbour per dir (-1 if outside)
vector<int> ea, eb;              // edge endpoints
vector<int> st, deg, unk, partner, clen;
int totalOn, pearlsTouched, numPearls;
int closed;
vector<pair<int *, int>> trail;
vector<int> q;
vector<char> inq;
long long nodes, nodeLimit, bestLen;
bool aborted;
bool randomOrder;

inline void setv(int &x, int v) { trail.push_back({&x, x}); x = v; }
inline void push(int c) { if (c >= 0 && !inq[c]) { inq[c] = 1; q.push_back(c); } }
inline void pushAround(int c) { push(c); for (int d = 0; d < 4; d++) push(nb[c][d]); }

bool setEdge(int e, int v) {
    if (st[e] == v) return true;
    if (st[e] != -1) return false;
    int a = ea[e], b = eb[e];
    setv(st[e], v);
    setv(unk[a], unk[a] - 1); setv(unk[b], unk[b] - 1);
    if (v == 1) {
        if (deg[a] >= 2 || deg[b] >= 2) return false;
        int endA = deg[a] == 0 ? a : partner[a], lenA = deg[a] == 0 ? 0 : clen[a];
        int endB = deg[b] == 0 ? b : partner[b], lenB = deg[b] == 0 ? 0 : clen[b];
        int touched = pearlsTouched + (deg[a] == 0 && pearl[a]) + (deg[b] == 0 && pearl[b]);
        if (endA == b) {  // closing a loop
            if (!(totalOn == lenA && touched == numPearls)) return false;
            setv(closed, 1);
        } else {
            setv(partner[endA], endB); setv(partner[endB], endA);
            setv(clen[endA], lenA + lenB + 1); setv(clen[endB], lenA + lenB + 1);
            push(endA); push(endB);
        }
        setv(deg[a], deg[a] + 1); setv(deg[b], deg[b] + 1);
        setv(totalOn, totalOn + 1);
        setv(pearlsTouched, touched);
        if (bestLen < INT_MAX && P4CFG::BB && totalOn >= bestLen) return false;
    }
    pushAround(a); pushAround(b);
    return true;
}

inline int E(int c, int d) { return ce[c][d]; }
inline int S(int c, int d) { return st[ce[c][d]]; }

bool checkCell(int c) {
    int dg = deg[c], uk = unk[c];
    if (dg > 2) return false;
    if (dg == 1 && uk == 0) return false;
    if (pearl[c] && dg + uk < 2) return false;
    if (dg == 2 && uk > 0) { for (int d = 0; d < 4; d++) if (S(c, d) == -1 && !setEdge(E(c, d), 0)) return false; }
    else if (dg == 1 && uk == 1) { for (int d = 0; d < 4; d++) if (S(c, d) == -1 && !setEdge(E(c, d), 1)) return false; }
    else if (dg == 0 && uk == 1 && !pearl[c]) { for (int d = 0; d < 4; d++) if (S(c, d) == -1 && !setEdge(E(c, d), 0)) return false; }
    else if (pearl[c] && dg + uk == 2 && uk > 0) { for (int d = 0; d < 4; d++) if (S(c, d) == -1 && !setEdge(E(c, d), 1)) return false; }
    if (pearl[c] == 'B') {
        for (int d = 0; d < 4; d++) {
            int o = (d + 2) & 3;
            int n1 = nb[c][d];
            int far = n1 >= 0 ? E(n1, d) : OFFE;   // edge continuing straight beyond the neighbour
            if (S(c, d) == 1) {
                if (!setEdge(E(c, o), 0)) return false;
                if (!setEdge(far, 1)) return false;
            }
            if (st[far] == 0 && !setEdge(E(c, d), 0)) return false;
            if (S(c, d) == 0 && !setEdge(E(c, o), 1)) return false;
        }
    } else if (pearl[c] == 'W') {
        // straight: E==W, N==S, horizontal != vertical
        for (int d = 0; d < 4; d++) {
            int o = (d + 2) & 3, p = (d + 1) & 3, po = (d + 3) & 3;
            int s = S(c, d);
            if (s == 1) { if (!setEdge(E(c, o), 1) || !setEdge(E(c, p), 0) || !setEdge(E(c, po), 0)) return false; }
            if (s == 0) { if (!setEdge(E(c, o), 0) || !setEdge(E(c, p), 1) || !setEdge(E(c, po), 1)) return false; }
        }
        // must turn in at least one neighbour along the loop
        for (int d = 0; d < 4; d++) {
            int o = (d + 2) & 3;
            if (S(c, d) != 1) continue;
            int n1 = nb[c][d], n2 = nb[c][o];
            int far1 = E(n1, d), far2 = E(n2, o);
            if (st[far1] == 1 && !setEdge(far2, 0)) return false;
            if (st[far2] == 1 && !setEdge(far1, 0)) return false;
        }
    }
    // no premature loop: chain end next to its partner
    if (deg[c] == 1) {
        int p = partner[c];
        for (int d = 0; d < 4; d++) if (nb[c][d] == p && S(c, d) == -1) {
            bool okClose = totalOn == clen[c] && pearlsTouched == numPearls;
            if (!okClose && !setEdge(E(c, d), 0)) return false;
        }
    }
    return true;
}

bool propagate() {
    while (!q.empty()) {
        int c = q.back(); q.pop_back(); inq[c] = 0;
        if (!checkCell(c)) { for (int x : q) inq[x] = 0; q.clear(); return false; }
    }
    return true;
}

void undo(size_t sz) { while (trail.size() > sz) { *trail.back().first = trail.back().second; trail.pop_back(); } }

bool finish() {  // loop closed: everything else off
    for (int e = 0; e < NE; e++) if (st[e] == -1 && !setEdge(e, 0)) return false;
    return propagate();
}

bool dfs() {
    if (!propagate()) return false;
    if (closed) return finish();
    if (++nodes > nodeLimit) { aborted = true; return false; }
    if ((nodes & 1023) == 0 && elapsed() > P4CFG::HARD_STOP_S) { aborted = true; return false; }
    // branch cell: chain end with fewest unknown edges; else a pearl with unknown edges
    int bc = -1;
    long long bu = LLONG_MIN;
    for (int c = 0; c < NC; c++) if (deg[c] == 1 && unk[c] > 0) {
        // FOCUS: 1 = extend the longest chain (keeps one path heading to its target),
        //        0 = chain end with the fewest unknown edges
        long long u = P4CFG::FOCUS ? (long long)clen[c] * 16 - unk[c] * 4 : -(long long)unk[c] * 4;
        if (randomOrder) u += (long long)(rnd() & 3);
        if (u > bu || bc < 0) { bu = u; bc = c; }
    }
    if (bc < 0) for (int c = 0; c < NC; c++) if (pearl[c] && unk[c] > 0) { bc = c; break; }
    if (bc < 0) return false;
    int ds[4], k = 0;
    for (int d = 0; d < 4; d++) if (S(bc, d) == -1) ds[k++] = d;
    // order: towards the nearest untouched pearl, or the chain's partner end once all are touched
    {
        int br = bc / C, bcc = bc % C, tr = -1, tc = -1, bestD = INT_MAX;
        if (pearlsTouched < numPearls) {
            for (int c = 0; c < NC; c++) if (pearl[c] && deg[c] == 0) {
                int dd = abs(c / C - br) + abs(c % C - bcc);
                if (dd < bestD) { bestD = dd; tr = c / C; tc = c % C; }
            }
        } else if (deg[bc] == 1) { tr = partner[bc] / C; tc = partner[bc] % C; }
        double key[4];
        for (int i = 0; i < k; i++) {
            int n1 = nb[bc][ds[i]];
            key[i] = tr >= 0 ? abs(n1 / C - tr) + abs(n1 % C - tc) : 0;
            if (randomOrder) key[i] += P4CFG::ORDER_NOISE * (rnd() >> 11) * (1.0 / 9007199254740992.0);
        }
        for (int i = 1; i < k; i++) for (int j = i; j > 0 && key[j] < key[j - 1]; j--) { swap(key[j], key[j - 1]); swap(ds[j], ds[j - 1]); }
    }
    // try each unknown edge ON in turn (others implied off by degree when bc is a chain end)
    for (int i = 0; i < k; i++) {
        size_t sz = trail.size();
        bool ok = setEdge(E(bc, ds[i]), 1);
        // previous options are off in this branch
        for (int j = 0; ok && j < i; j++) ok = setEdge(E(bc, ds[j]), 0);
        if (ok && dfs()) return true;
        undo(sz);
        if (aborted) return false;
    }
    if (deg[bc] == 0) {  // a pearl branch could also be... pearls need degree 2, so no "all off"
        return false;
    }
    return false;
}

vector<string> render() {
    vector<string> out(R, string(C, '.'));
    for (int c = 0; c < NC; c++) {
        if (deg[c] != 2) continue;
        bool n = S(c, 0) == 1, e = S(c, 1) == 1, s = S(c, 2) == 1, w = S(c, 3) == 1;
        char ch = '.';
        if (n && s) ch = '|'; else if (e && w) ch = '-';
        else if (n && e) ch = 'L'; else if (n && w) ch = 'J';
        else if (s && e) ch = 'F'; else if (s && w) ch = '7';
        out[c / C][c % C] = ch;
    }
    return out;
}

vector<string> run(vector<string> grid) {
    T0 = chrono::steady_clock::now();
    rs = 88172645463325252ULL;
    for (auto &row : grid) {
        string t;
        for (char ch : row) if (ch == '.' || ch == 'W' || ch == 'B') t += ch;
        row = t;
    }
    while (!grid.empty() && grid.back().empty()) grid.pop_back();
    R = grid.size(); C = 0;
    for (auto &row : grid) C = max(C, (int)row.size());
    for (auto &row : grid) row.resize(C, '.');
    NC = R * C;
    pearl.assign(NC, 0);
    numPearls = 0;
    for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) if (grid[r][c] != '.') { pearl[r * C + c] = grid[r][c]; numPearls++; }
    // edges
    ce.assign(NC, {}); nb.assign(NC, {});
    ea.clear(); eb.clear();
    vector<int> eid(NC * 4, -1);
    for (int r = 0; r < R; r++) for (int c = 0; c < C; c++) {
        int id = r * C + c;
        for (int d = 0; d < 4; d++) {
            int rr = r + DR[d], cc = c + DC[d];
            nb[id][d] = (rr >= 0 && rr < R && cc >= 0 && cc < C) ? rr * C + cc : -1;
        }
    }
    for (int c = 0; c < NC; c++) for (int d = 1; d <= 2; d++) if (nb[c][d] >= 0) {
        int e = ea.size(); ea.push_back(c); eb.push_back(nb[c][d]);
        eid[c * 4 + d] = e; eid[nb[c][d] * 4 + ((d + 2) & 3)] = e;
    }
    NE = ea.size(); OFFE = NE;
    for (int c = 0; c < NC; c++) for (int d = 0; d < 4; d++) ce[c][d] = eid[c * 4 + d] >= 0 ? eid[c * 4 + d] : OFFE;
    // initial state
    st.assign(NE + 1, -1); st[OFFE] = 0;
    deg.assign(NC, 0); unk.assign(NC, 0);
    for (int c = 0; c < NC; c++) for (int d = 0; d < 4; d++) if (ce[c][d] != OFFE) unk[c]++;
    partner.assign(NC, 0); clen.assign(NC, 0);
    for (int c = 0; c < NC; c++) partner[c] = c;
    totalOn = 0; pearlsTouched = 0; closed = 0;
    trail.clear(); q.clear(); inq.assign(NC, 0);
    for (int c = 0; c < NC; c++) push(c);
    bestLen = INT_MAX;
    vector<string> best(R, string(C, '.'));
    bool ok0 = propagate();
    size_t base = trail.size();
    if (ok0) {
        // first loop: deterministic DFS, no node limit (up to the deadline via restarts)
        randomOrder = false; nodes = 0; nodeLimit = 200000; aborted = false;
        bool found = dfs();
        while (!found && elapsed() < P4CFG::FIRST_DEADLINE_S && elapsed() < P4CFG::HARD_STOP_S) {
            undo(base);
            randomOrder = true; nodes = 0; aborted = false;
            nodeLimit = min<long long>(nodeLimit * 2, 50000000);
            found = dfs();
        }
        if (found) { bestLen = totalOn; best = render(); }
        undo(base);
        // improvement: randomised restarts with branch and bound on length
        randomOrder = true;
        while (found && elapsed() < P4CFG::IMPROVE_UNTIL_S) {
            nodes = 0; nodeLimit = P4CFG::RESTART_NODES; aborted = false;
            if (dfs() && totalOn < bestLen) { bestLen = totalOn; best = render(); }
            undo(base);
        }
    }
#ifdef LOCAL
    cerr << "R " << R << " C " << C << " pearls " << numPearls << " len " << bestLen << " t " << elapsed() << "\n";
#endif
    return best;
}
}  // namespace p4b

vector<string> solve(vector<string> grid) { return p4b::run(grid); }

#ifdef LOCAL
int main() {
    if (getenv("IMP")) P4CFG::IMPROVE_UNTIL_S = atof(getenv("IMP"));
    if (getenv("ON")) P4CFG::ORDER_NOISE = atof(getenv("ON"));
    if (getenv("FO")) P4CFG::FOCUS = atoi(getenv("FO"));
    if (getenv("RN")) P4CFG::RESTART_NODES = atoll(getenv("RN"));
    int R;
    string line;
    if (!getline(cin, line)) return 0;
    R = stoi(line);
    vector<string> g(R);
    for (auto &row : g) getline(cin, row);
    auto out = solve(g);
    for (auto &row : out) cout << row << "\n";
}
#endif
