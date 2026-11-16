// Simulated annealing skeleton (C++). Fill in State, initial(), score(), tweak().
// Build: g++ -std=c++17 -O2 -o solve solve.cpp
// Same idea as anneal.py, typically 10-100x more iterations in the same time.
// Speed-up for later: change the state in place and undo rejected moves instead of copying.
#include <bits/stdc++.h>
using namespace std;

struct Config {
    double time_budget_s = 8.0;  // about 80% of the problem's time limit
    double start_temp = 1.0;     // roughly the size of a typical score change early on
    double end_temp = 0.01;
    uint64_t seed = 1;
    int clock_check_every = 256; // reading the clock is slow; check it every N iterations
};

struct Input { /* TODO: problem data */ };
struct State { /* TODO: your answer */ };

mt19937_64 rng;

State initial(const Input& in) { return State{}; }             // TODO: simple VALID answer
double score(const Input& in, const State& s) { return 0.0; }  // TODO: higher is better
State tweak(const Input& in, const State& s) { return s; }     // TODO: one small random change

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    const Config cfg;
    rng.seed(cfg.seed);

    Input in;  // TODO: read stdin into `in`

    const auto t0 = chrono::steady_clock::now();
    auto elapsed = [&] { return chrono::duration<double>(chrono::steady_clock::now() - t0).count(); };
    uniform_real_distribution<double> unit(0.0, 1.0);

    State cur = initial(in), best = cur;
    double cur_s = score(in, cur), best_s = cur_s;
    long long iters = 0;
    double t = 0.0;
    while (true) {
        if (iters % cfg.clock_check_every == 0 && (t = elapsed()) >= cfg.time_budget_s) break;
        const double frac = t / cfg.time_budget_s;
        const double temp = cfg.start_temp * pow(cfg.end_temp / cfg.start_temp, frac);
        State cand = tweak(in, cur);
        const double cand_s = score(in, cand);
        if (cand_s >= cur_s || unit(rng) < exp((cand_s - cur_s) / temp)) {
            cur = std::move(cand);
            cur_s = cand_s;
            if (cur_s > best_s) { best = cur; best_s = cur_s; }
        }
        ++iters;
    }
    cerr << "iters=" << iters << " best=" << best_s << "\n";
    // TODO: print `best` in the required output format to stdout
    return 0;
}
