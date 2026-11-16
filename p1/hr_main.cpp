// HackerRank's real hidden code for p1, copied exactly. Only changes: your solution file is
// #included where the stub function was, and OUTPUT_PATH defaults to stdout for local runs.
// Build with:  ./hrbuild.sh p1 p1/<you>/solve.cpp p1/<you>/solve
#include <bits/stdc++.h>

using namespace std;

string ltrim(const string &);
string rtrim(const string &);

#include HR_SOLUTION

int main()
{
    ofstream fout(getenv("OUTPUT_PATH") ? getenv("OUTPUT_PATH") : "/dev/stdout");

    string squares_count_temp;
    getline(cin, squares_count_temp);

    int squares_count = stoi(ltrim(rtrim(squares_count_temp)));

    vector<int> squares(squares_count);

    for (int i = 0; i < squares_count; i++) {
        string squares_item_temp;
        getline(cin, squares_item_temp);

        int squares_item = stoi(ltrim(rtrim(squares_item_temp)));

        squares[i] = squares_item;
    }

    vector<vector<int>> result = pack_squares(squares);

    for (size_t i = 0; i < result.size(); i++) {
        for (size_t j = 0; j < result[i].size(); j++) {
            fout << result[i][j];

            if (j != result[i].size() - 1) {
                fout << " ";
            }
        }

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
