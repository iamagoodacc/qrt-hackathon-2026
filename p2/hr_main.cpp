// HackerRank's real hidden code for p2, copied exactly. Only changes: your solution file is
// #included where the stub function was, and OUTPUT_PATH defaults to stdout for local runs.
// Build with:  ./hrbuild.sh p2 p2/<you>/solve.cpp p2/<you>/solve
// NOTE the output format: each grid row is printed with SPACES between letters, e.g. "A C E".
#include <bits/stdc++.h>

using namespace std;

string ltrim(const string &);
string rtrim(const string &);

#include HR_SOLUTION

int main()
{
    ofstream fout(getenv("OUTPUT_PATH") ? getenv("OUTPUT_PATH") : "/dev/stdout");

    string required_words_count_temp;
    getline(cin, required_words_count_temp);

    int required_words_count = stoi(ltrim(rtrim(required_words_count_temp)));

    vector<string> required_words(required_words_count);

    for (int i = 0; i < required_words_count; i++) {
        string required_words_item;
        getline(cin, required_words_item);

        required_words[i] = required_words_item;
    }

    string forbidden_words_count_temp;
    getline(cin, forbidden_words_count_temp);

    int forbidden_words_count = stoi(ltrim(rtrim(forbidden_words_count_temp)));

    vector<string> forbidden_words(forbidden_words_count);

    for (int i = 0; i < forbidden_words_count; i++) {
        string forbidden_words_item;
        getline(cin, forbidden_words_item);

        forbidden_words[i] = forbidden_words_item;
    }

    vector<vector<char>> result = generate_grid(required_words, forbidden_words);

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
