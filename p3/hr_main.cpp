// HackerRank's real hidden code for p3, copied exactly. Only changes: your solution file is
// #included where the stub function was, and OUTPUT_PATH defaults to stdout for local runs.
// Build with:  ./hrbuild.sh p3 p3/<you>/solve.cpp p3/<you>/solve
// NOTE: there is NO black-box function in here. Input = one float t; output = your floats, one per line.
#include <bits/stdc++.h>

using namespace std;

string ltrim(const string &);
string rtrim(const string &);

#include HR_SOLUTION

int main()
{
    ofstream fout(getenv("OUTPUT_PATH") ? getenv("OUTPUT_PATH") : "/dev/stdout");

    string t_temp;
    getline(cin, t_temp);

    float t = stof(ltrim(rtrim(t_temp)));

    vector<float> result = probe(t);

    for (size_t i = 0; i < result.size(); i++) {
        fout << result[i];

        if (i != result.size() - 1) {
            fout << "\n";
        }
    }

    fout << "\n";

    fout.close();

    return 0;
}

string ltrim(const string &str) {
    string s(str);

    s.erase(
        s.begin(),
        find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace)))
    );

    return s;
}

string rtrim(const string &str) {
    string s(str);

    s.erase(
        find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(),
        s.end()
    );

    return s;
}
