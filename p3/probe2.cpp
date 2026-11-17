#include <bits/stdc++.h>
using namespace std;
// p3 EXPERIMENT (Run Code only, do NOT submit): 4 combos cycling over the 51 cases (k = t/0.02):
//   k%4==0: x=0.03 N=1    k%4==1: x=0.08 N=1    k%4==2: x=0.03 N=1000    k%4==3: x=0.08 N=1000
namespace P3CFG { float XA = 0.03f, XB = 0.08f; int NBIG = 1000; }
vector<float> probe(float t) {
    using namespace P3CFG;
    int k = (int)lround(t / 0.02f);
    float x = (k % 2 == 0) ? XA : XB;
    int n = (k % 4 >= 2) ? NBIG : 1;
    return vector<float>(n, x);
}
