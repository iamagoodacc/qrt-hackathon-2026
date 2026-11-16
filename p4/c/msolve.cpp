// p4-c: Masyu solver. Edge variables (unknown/off/on) with full propagation:
//  - cell degree 0 or 2 (pearls exactly 2)
//  - black: turn (one of N/S, one of E/W) and each used arm continues straight for 2 cells
//  - white: straight through, and not straight at both ends
//  - no premature sub-loop: fragment ends track their partner; closing is only allowed when the
//    fragment holds every on-edge and every pearl
// Failed-literal probing at the root, then DFS branching at the fragment end with fewest options.
#include <bits/stdc++.h>
using namespace std;

namespace MCFG {
double SOLVE_LIMIT_S = 5.5;      // never search for a first loop beyond this (HackerRank is slower)
int PROBE_ROUNDS = 50;           // max failed-literal probing sweeps at the root
double RESTART_NODES0 = 50000;   // node limit of the first DFS run
double RESTART_GROWTH = 2.0;     // node limit multiplier per restart
int CONN_CHECK = 2;              // prune DFS nodes whose pearls/loop cells are disconnected
double PHASE_A_S = 0.3;          // plain-DFS phase length before switching to probing DFS
int PROBE_FRONTIER = 1;          // DFS-node probing only on edges at fragment ends / unfinished pearls
int PROBE_CONN = 0;              // also run the biconnectivity check inside each probe
int PROBE_DEPTH = 0;              // also probe (one sweep) at DFS nodes shallower than this depth
}

namespace mz {
int R, C, NC, NE, OFFE, numPearls;
vector<char> pearl;
vector<array<int, 4>> nb, ce;    // neighbour cell / edge id per direction N,E,S,W
vector<int> ea, eb;
vector<int> st, deg, unk, partner, flen, inq;
int totalOn, touched, closedF;
struct TR { int *p; int old; };
vector<TR> trail;
vector<int> q;
vector<vector<int>> pearlsNear;  // pearls within Manhattan distance 2 of a cell
chrono::steady_clock::time_point T0;
double elapsed() { return chrono::duration<double>(chrono::steady_clock::now() - T0).count(); }
long long nodes, nodeLimit = LLONG_MAX;
double curLimit; int curProbeDepth;
bool timeUp, aborted, randomize;
uint64_t rs = 0x9E3779B97F4A7C15ULL;
inline uint64_t rnd() { rs ^= rs << 13; rs ^= rs >> 7; rs ^= rs << 17; return rs; }

inline void setv(int &x, int v) { trail.push_back({&x, x}); x = v; }
inline void undo(size_t mark) { while (trail.size() > mark) { *trail.back().p = trail.back().old; trail.pop_back(); } }
inline void enq(int c) { if (!inq[c]) { inq[c] = 1; q.push_back(c); } }

bool setEdge(int e, int v) {
    if (e == OFFE) return v == 0;
    int s = st[e];
    if (s == v) return true;
    if (s != -1) return false;
    setv(st[e], v);
    int a = ea[e], b = eb[e];
    setv(unk[a], unk[a] - 1); setv(unk[b], unk[b] - 1);
    if (v == 1) {
        if (deg[a] >= 2 || deg[b] >= 2) return false;
        if (deg[a] == 1 && deg[b] == 1 && partner[a] == b) {
            if (flen[a] != totalOn || touched != numPearls) return false;
            setv(closedF, 1);
        } else {
            int pa = deg[a] ? partner[a] : a, pb = deg[b] ? partner[b] : b;
            int nl = (deg[a] ? flen[a] : 0) + (deg[b] ? flen[b] : 0) + 1;
            setv(partner[pa], pb); setv(partner[pb], pa); setv(flen[pa], nl); setv(flen[pb], nl);
        }
        if (pearl[a] && deg[a] == 0) setv(touched, touched + 1);
        if (pearl[b] && deg[b] == 0) setv(touched, touched + 1);
        setv(deg[a], deg[a] + 1); setv(deg[b], deg[b] + 1);
        setv(totalOn, totalOn + 1);
    }
    enq(a); enq(b);
    for (int p : pearlsNear[a]) enq(p);
    for (int p : pearlsNear[b]) enq(p);
    return true;
}
inline int S(int e) { return e == OFFE ? 0 : st[e]; }
inline bool adjacentDir(int a, int b, int &d) {
    for (d = 0; d < 4; d++) if (nb[a][d] == b) return true;
    return false;
}

bool checkCell(int c) {
    if (deg[c] > 2) return false;
    if (deg[c] == 2) {
        if (unk[c]) for (int d = 0; d < 4; d++) if (S(ce[c][d]) == -1 && !setEdge(ce[c][d], 0)) return false;
    } else if (deg[c] == 1) {
        if (unk[c] == 0) return false;
        if (unk[c] == 1) { for (int d = 0; d < 4; d++) if (S(ce[c][d]) == -1) { if (!setEdge(ce[c][d], 1)) return false; break; } }
        else {
            int p = partner[c], d;
            if (deg[c] == 1 && deg[p] == 1 && adjacentDir(c, p, d) && S(ce[c][d]) == -1 && !(flen[c] == totalOn && touched == numPearls))
                if (!setEdge(ce[c][d], 0)) return false;
        }
    } else {
        if (pearl[c]) {
            if (unk[c] < 2) return false;
            if (unk[c] == 2) for (int d = 0; d < 4; d++) if (S(ce[c][d]) == -1 && !setEdge(ce[c][d], 1)) return false;
        } else if (unk[c] == 1) {
            for (int d = 0; d < 4; d++) if (S(ce[c][d]) == -1) { if (!setEdge(ce[c][d], 0)) return false; break; }
        }
    }
    if (pearl[c] == 'B') {
        for (int d = 0; d < 4; d++) {
            int e1 = ce[c][d];
            if (e1 == OFFE) continue;
            int n1 = nb[c][d];
            int e2 = ce[n1][d];
            if (S(e1) == 1) { if (!setEdge(ce[c][d ^ 2], 0) || !setEdge(e2, 1)) return false; }
            if (S(e2) == 0 || pearl[n1] == 'B' || S(ce[n1][d ^ 1]) == 1 || S(ce[n1][d ^ 3]) == 1)
                if (!setEdge(e1, 0)) return false;
        }
        for (int d = 0; d < 2; d++) {
            int a = ce[c][d], b = ce[c][d + 2];
            if (S(a) == 0 && !setEdge(b, 1)) return false;
            if (S(b) == 0 && !setEdge(a, 1)) return false;
            if (S(a) == 1 && !setEdge(b, 0)) return false;
            if (S(b) == 1 && !setEdge(a, 0)) return false;
        }
    } else if (pearl[c] == 'W') {
        for (int ax = 0; ax < 2; ax++) {           // ax 0: vertical (N,S), ax 1: horizontal (E,W)
            int a = ce[c][ax], b = ce[c][ax + 2], pa = ce[c][ax ^ 1], pb = ce[c][(ax ^ 1) + 2];
            if (S(a) != -1 && !setEdge(b, S(a))) return false;
            if (S(b) != -1 && !setEdge(a, S(b))) return false;
            if (S(a) == 1 && (!setEdge(pa, 0) || !setEdge(pb, 0))) return false;
            if (S(a) == 0 && (!setEdge(pa, 1) || !setEdge(pb, 1))) return false;
            // turn at one end along this axis
            if (S(a) != 0) {
                int n1 = nb[c][ax], n2 = nb[c][ax + 2];
                if (n1 < 0 || n2 < 0) { if (!setEdge(a, 0)) return false; continue; }
                int f1 = ce[n1][ax], f2 = ce[n2][ax + 2];
                if (S(f1) == 1 && S(f2) == 1) { if (!setEdge(a, 0)) return false; }
                else if (S(a) == 1) {
                    if (S(f1) == 1 && !setEdge(f2, 0)) return false;
                    if (S(f2) == 1 && !setEdge(f1, 0)) return false;
                }
            }
        }
    }
    return true;
}

bool propagate() {
    while (true) {
        while (!q.empty()) {
            int c = q.back(); q.pop_back(); inq[c] = 0;
            if (!checkCell(c)) { for (int x : q) inq[x] = 0; q.clear(); return false; }
        }
        if (!closedF) return true;
        bool any = false;
        for (int e = 0; e < NE; e++) if (st[e] == -1) { any = true; if (!setEdge(e, 0)) { for (int x : q) inq[x] = 0; q.clear(); return false; } }
        if (!any && q.empty()) return true;
    }
}

vector<string> render() {
    vector<string> out(R, string(C, '.'));
    for (int c = 0; c < NC; c++) {
        int m = 0;
        for (int d = 0; d < 4; d++) if (S(ce[c][d]) == 1) m |= 1 << d;
        char ch = '.';
        if (m == 5) ch = '|'; else if (m == 10) ch = '-'; else if (m == 3) ch = 'L';
        else if (m == 9) ch = 'J'; else if (m == 6) ch = 'F'; else if (m == 12) ch = '7';
        out[c / C][c % C] = ch;
    }
    return out;
}
// full independent validation of a rendered loop
bool validLoop(const vector<string> &out, const vector<string> &g) {
    int Rr = g.size(), Cc = g[0].size();
    auto mk = [](char ch) { switch (ch) { case '|': return 5; case '-': return 10; case 'L': return 3; case 'J': return 9; case 'F': return 6; case '7': return 12; } return 0; };
    const int DR[4] = {-1, 0, 1, 0}, DC[4] = {0, 1, 0, -1};
    int used = 0, sr = -1, sc = -1;
    auto mAt = [&](int r, int c) { return (r < 0 || r >= Rr || c < 0 || c >= Cc) ? 0 : mk(out[r][c]); };
    auto straight = [](int m) { return m == 5 || m == 10; };
    for (int r = 0; r < Rr; r++) for (int c = 0; c < Cc; c++) {
        int m = mAt(r, c);
        if (g[r][c] != '.' && !m) return false;
        if (!m) continue;
        used++; sr = r; sc = c;
        int nbs[2][2], k = 0;
        for (int d = 0; d < 4; d++) if (m >> d & 1) {
            int rr = r + DR[d], cc = c + DC[d];
            if (!(mAt(rr, cc) >> ((d + 2) & 3) & 1)) return false;
            nbs[k][0] = rr; nbs[k][1] = cc; k++;
        }
        bool t0 = !straight(mAt(nbs[0][0], nbs[0][1])), t1 = !straight(mAt(nbs[1][0], nbs[1][1]));
        if (g[r][c] == 'W' && (!straight(m) || !(t0 || t1))) return false;
        if (g[r][c] == 'B' && (straight(m) || t0 || t1)) return false;
    }
    if (!used) return false;
    // single loop: walk from (sr,sc)
    int r = sr, c = sc, pd = -1, steps = 0;
    do {
        int m = mAt(r, c), nd = -1;
        for (int d = 0; d < 4; d++) if ((m >> d & 1) && d != ((pd + 2) & 3)) { nd = d; break; }
        if (pd == -1) { for (int d = 0; d < 4; d++) if (m >> d & 1) { nd = d; break; } }
        r += DR[nd]; c += DC[nd]; pd = nd; steps++;
    } while ((r != sr || c != sc) && steps <= used);
    return steps == used;
}

vector<string> gridIn, solution;
// Tarjan: a cell whose removal separates required cells (pearls / loop cells) => contradiction
vector<int> tdisc, tlow, tsub, tpe, tpv, tdi, tstk;
bool biconnOK() {
    if ((int)tdisc.size() != NC) { tdisc.assign(NC, 0); tlow.assign(NC, 0); tsub.assign(NC, 0); tpe.assign(NC, 0); tpv.assign(NC, 0); tdi.assign(NC, 0); }
    int root = -1, totalReq = 0;
    for (int c = 0; c < NC; c++) if (deg[c] > 0 || pearl[c]) { totalReq++; if (root < 0) root = c; }
    if (root < 0) return true;
    for (int c = 0; c < NC; c++) tdisc[c] = 0;
    int timer = 0, rootReqChildren = 0;
    tstk.clear();
    tdisc[root] = tlow[root] = ++timer; tsub[root] = 1; tpe[root] = -2; tpv[root] = -1; tdi[root] = 0;
    tstk.push_back(root);
    while (!tstk.empty()) {
        int v = tstk.back();
        if (tdi[v] < 4) {
            int d = tdi[v]++;
            int e = ce[v][d];
            if (S(e) == 0 || e == tpe[v]) continue;
            int w = nb[v][d];
            if (!tdisc[w]) {
                tdisc[w] = tlow[w] = ++timer; tsub[w] = (deg[w] > 0 || pearl[w]) ? 1 : 0;
                tpe[w] = e; tpv[w] = v; tdi[w] = 0; tstk.push_back(w);
            } else tlow[v] = min(tlow[v], tdisc[w]);
        } else {
            tstk.pop_back();
            int p = tpv[v];
            if (p < 0) continue;
            tlow[p] = min(tlow[p], tlow[v]);
            if (tlow[v] >= tdisc[p] && tsub[v] > 0) {
                if (p != root) return false;
                if (++rootReqChildren >= 2) return false;
            }
            tsub[p] += tsub[v];
        }
    }
    return tsub[root] == totalReq;
}
vector<int> bfsMark, bfsQ; int bfsStamp = 0;
// all pearls and loop cells must lie in one component of the graph of non-off edges
bool connected() {
    if (bfsMark.size() != (size_t)NC) bfsMark.assign(NC, 0);
    int start = -1;
    for (int c = 0; c < NC; c++) if (deg[c] > 0 || pearl[c]) { start = c; break; }
    if (start < 0) return true;
    bfsStamp++; bfsQ.clear(); bfsQ.push_back(start); bfsMark[start] = bfsStamp;
    for (size_t h = 0; h < bfsQ.size(); h++) {
        int c = bfsQ[h];
        for (int d = 0; d < 4; d++) { int e = ce[c][d]; if (S(e) != 0) { int n = nb[c][d]; if (bfsMark[n] != bfsStamp) { bfsMark[n] = bfsStamp; bfsQ.push_back(n); } } }
    }
    for (int c = 0; c < NC; c++) if ((deg[c] > 0 || pearl[c]) && bfsMark[c] != bfsStamp) return false;
    return true;
}

vector<int> probeList;
bool probeSweep(bool &changed, bool frontier = false) {
    changed = false;
    probeList.clear();
    if (frontier) {
        for (int c = 0; c < NC; c++) if ((deg[c] == 1 || (pearl[c] && deg[c] < 2)) && unk[c])
            for (int d = 0; d < 4; d++) { int e = ce[c][d]; if (S(e) == -1 && (d == 1 || d == 2 || !(deg[nb[c][d]] == 1 || (pearl[nb[c][d]] && deg[nb[c][d]] < 2)))) probeList.push_back(e); }
    } else for (int e = 0; e < NE; e++) probeList.push_back(e);
    for (int e : probeList) {
        if (st[e] != -1) continue;
        if ((e & 63) == 0 && elapsed() > curLimit) { timeUp = true; return true; }
        size_t mark = trail.size();
        bool ok1 = setEdge(e, 1) && propagate() && (!MCFG::PROBE_CONN || closedF || biconnOK());
        if (ok1 && closedF) { auto o = render(); if (validLoop(o, gridIn)) { solution = o; } ok1 = !solution.empty(); }
        undo(mark);
        if (!solution.empty()) return true;
        if (!ok1) { if (!setEdge(e, 0) || !propagate()) return false; changed = true; continue; }
        bool ok0 = setEdge(e, 0) && propagate() && (!MCFG::PROBE_CONN || biconnOK());
        undo(mark);
        if (!ok0) { if (!setEdge(e, 1) || !propagate()) return false; changed = true; }
    }
    return true;
}

bool dfs(int depth) {
    if ((++nodes & 255) == 0 && elapsed() > curLimit) timeUp = true;
    if (nodes > nodeLimit) aborted = true;
    if (timeUp || aborted) return false;
    if (closedF) {
        auto o = render();
        if (validLoop(o, gridIn)) { solution = o; return true; }
        return false;
    }
    size_t mark0 = trail.size();
    if (MCFG::CONN_CHECK == 1 && !connected()) return false;
    if (MCFG::CONN_CHECK == 2 && !biconnOK()) return false;
    if (depth < curProbeDepth) {
        bool ch;
        if (!probeSweep(ch, MCFG::PROBE_FRONTIER)) { undo(mark0); return false; }
        if (!solution.empty()) return true;
        if (closedF) { bool r = dfs(depth + 1); if (!r) undo(mark0); return r; }
    }
    // branch cell: fragment end with fewest unknown edges
    int best = -1, bu = 9, ties = 0;
    if (!randomize) {
        for (int c = 0; c < NC; c++) if (deg[c] == 1 && unk[c] < bu) { bu = unk[c]; best = c; if (bu <= 2) break; }
    } else {
        for (int c = 0; c < NC; c++) if (deg[c] == 1) {
            if (unk[c] < bu) { bu = unk[c]; best = c; ties = 1; }
            else if (unk[c] == bu && rnd() % (++ties) == 0) best = c;
        }
    }
    if (best < 0) {
        // no fragment yet: branch on an unknown edge of a pearl (on, then off)
        int e = -1;
        for (int c = 0; c < NC && e < 0; c++) if (pearl[c]) for (int d = 0; d < 4; d++) if (S(ce[c][d]) == -1) { e = ce[c][d]; break; }
        if (e < 0) for (int x = 0; x < NE; x++) if (st[x] == -1) { e = x; break; }
        if (e < 0) return false;
        for (int v = 1; v >= 0; v--) {
            size_t mark = trail.size();
            if (setEdge(e, v) && propagate() && dfs(depth + 1)) return true;
            undo(mark);
            if (timeUp || aborted) { undo(mark0); return false; }
        }
        undo(mark0);
        return false;
    }
    int opts[4], k = 0;
    for (int d = 0; d < 4; d++) if (S(ce[best][d]) == -1) opts[k++] = ce[best][d];
    if (randomize) for (int i = k - 1; i > 0; i--) swap(opts[i], opts[rnd() % (i + 1)]);
    for (int i = 0; i < k; i++) {
        size_t mark = trail.size();
        if (setEdge(opts[i], 1) && propagate() && dfs(depth + 1)) return true;
        undo(mark);
        if (timeUp || aborted) { undo(mark0); return false; }
        // this option failed: it must be off for the remaining options
        if (!setEdge(opts[i], 0) || !propagate()) { undo(mark0); return false; }
    }
    undo(mark0);
    return false;
}

// returns a valid loop or empty vector
vector<string> run(const vector<string> &grid, double t0offset) {
    T0 = chrono::steady_clock::now() - chrono::duration_cast<chrono::steady_clock::duration>(chrono::duration<double>(t0offset));
    curLimit = MCFG::SOLVE_LIMIT_S; curProbeDepth = 0;
    gridIn = grid; solution.clear(); timeUp = false; nodes = 0;
    R = grid.size(); C = grid[0].size(); NC = R * C;
    pearl.assign(NC, 0); numPearls = 0;
    for (int c = 0; c < NC; c++) { char ch = grid[c / C][c % C]; if (ch == 'W' || ch == 'B') { pearl[c] = ch; numPearls++; } }
    const int DR[4] = {-1, 0, 1, 0}, DC[4] = {0, 1, 0, -1};
    nb.assign(NC, {}); ce.assign(NC, {});
    ea.clear(); eb.clear();
    for (int r = 0; r < R; r++) for (int c = 0; c < C; c++)
        for (int d = 0; d < 4; d++) { int rr = r + DR[d], cc = c + DC[d]; nb[r * C + c][d] = (rr >= 0 && rr < R && cc >= 0 && cc < C) ? rr * C + cc : -1; }
    vector<int> eid(NC * 4, -1);
    for (int c = 0; c < NC; c++) for (int d = 1; d <= 2; d++) if (nb[c][d] >= 0) {
        int e = ea.size(); ea.push_back(c); eb.push_back(nb[c][d]);
        eid[c * 4 + d] = e; eid[nb[c][d] * 4 + ((d + 2) & 3)] = e;
    }
    NE = ea.size(); OFFE = NE;
    for (int c = 0; c < NC; c++) for (int d = 0; d < 4; d++) ce[c][d] = eid[c * 4 + d] >= 0 ? eid[c * 4 + d] : OFFE;
    st.assign(NE + 1, -1); st[OFFE] = 0;
    deg.assign(NC, 0); unk.assign(NC, 0); partner.assign(NC, 0); flen.assign(NC, 0); inq.assign(NC, 0);
    for (int c = 0; c < NC; c++) { partner[c] = c; for (int d = 0; d < 4; d++) if (ce[c][d] != OFFE) unk[c]++; }
    pearlsNear.assign(NC, {});
    for (int c = 0; c < NC; c++) {
        int r = c / C, cc = c % C;
        for (int dr = -2; dr <= 2; dr++) for (int dc = -2; dc <= 2; dc++) {
            if (abs(dr) + abs(dc) > 2) continue;
            int rr = r + dr, c2 = cc + dc;
            if (rr >= 0 && rr < R && c2 >= 0 && c2 < C && pearl[rr * C + c2]) pearlsNear[c].push_back(rr * C + c2);
        }
    }
    totalOn = 0; touched = 0; closedF = 0;
    trail.clear(); trail.reserve(1 << 16); q.clear();
    for (int c = 0; c < NC; c++) enq(c);
    if (!propagate()) return {};
    for (int round = 0; round < MCFG::PROBE_ROUNDS; round++) {
        bool ch;
        if (!probeSweep(ch)) return {};
        if (!solution.empty() || timeUp || !ch) break;
    }
    if (!solution.empty()) return solution;
    if (closedF) { auto o = render(); if (validLoop(o, gridIn)) return o; }
    // portfolio: plain DFS (fast on easy/dense puzzles), then DFS with frontier probing at every node
    size_t root = trail.size();
    for (int phase = 0; phase < 2; phase++) {
        curProbeDepth = phase == 0 ? MCFG::PROBE_DEPTH : 1000000000;
        curLimit = phase == 0 ? min(MCFG::SOLVE_LIMIT_S, elapsed() + MCFG::PHASE_A_S) : MCFG::SOLVE_LIMIT_S;
        timeUp = false;
        double lim = MCFG::RESTART_NODES0;
        for (int restart = 0; !timeUp; restart++) {
            randomize = restart > 0;
            nodes = 0; nodeLimit = (long long)lim; aborted = false;
            if (dfs(0)) return solution;
            undo(root);
            if (!aborted) break;
            lim *= MCFG::RESTART_GROWTH;
        }
        if (elapsed() > MCFG::SOLVE_LIMIT_S) break;
    }
    return {};
}
}  // namespace mz

#ifndef MZ_NO_SOLVE
vector<string> solve(vector<string> grid) {
    for (auto &row : grid) { string t; for (char ch : row) if (ch == '.' || ch == 'W' || ch == 'B') t += ch; row = t; }
    while (!grid.empty() && grid.back().empty()) grid.pop_back();
    size_t C = 0; for (auto &r : grid) C = max(C, r.size());
    for (auto &r : grid) r.resize(C, '.');
#ifdef MZDBG
    if (getenv("PD")) MCFG::PROBE_DEPTH = atoi(getenv("PD"));
    if (getenv("CC")) MCFG::CONN_CHECK = atoi(getenv("CC"));
    if (getenv("PA")) MCFG::PHASE_A_S = atof(getenv("PA"));
    if (getenv("PF")) MCFG::PROBE_FRONTIER = atoi(getenv("PF"));
    if (getenv("PC")) MCFG::PROBE_CONN = atoi(getenv("PC"));
    if (getenv("SL")) MCFG::SOLVE_LIMIT_S = atof(getenv("SL"));
    if (getenv("RN")) MCFG::RESTART_NODES0 = atof(getenv("RN"));
    if (getenv("RG")) MCFG::RESTART_GROWTH = atof(getenv("RG"));
#endif
    auto out = mz::run(grid, 0.0);
#ifdef MZDBG
    cerr << "nodes=" << mz::nodes << " t=" << mz::elapsed() << " found=" << !out.empty() << "\n";
#endif
    if (out.empty()) return vector<string>(grid.size(), string(C, '.'));
    return out;
}
#endif
