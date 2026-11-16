// HackerRank's real hidden code for p4, copied exactly. Only changes: your solution file is
// #included where the stub function was, and OUTPUT_PATH defaults to stdout for local runs.
// Build with:  ./hrbuild.sh p4 p4/<you>/solve.cpp p4/<you>/solve
#include <bits/stdc++.h>

using namespace std;

string ltrim(const string &);
string rtrim(const string &);

#include HR_SOLUTION

int main()
{
    ofstream fout(getenv("OUTPUT_PATH") ? getenv("OUTPUT_PATH") : "/dev/stdout");

    string grid_count_temp;
    getline(cin, grid_count_temp);

    int grid_count = stoi(ltrim(rtrim(grid_count_temp)));

    vector<string> grid(grid_count);

    for (int i = 0; i < grid_count; i++) {
        string grid_item;
        getline(cin, grid_item);

        grid[i] = grid_item;
    }

    vector<string> result = solve(grid);

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
