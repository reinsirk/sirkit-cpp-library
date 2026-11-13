// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <string>
#include <vector>

int main() {
    int n{};
    verify_in >> n;
    std::vector<std::string> s(n), t(n);
    for (auto& row : s) verify_in >> row;
    for (auto& row : t) verify_in >> row;
    int answer = n * n;
    for (int turns = 0; turns < 4; ++turns) {
        int cost = turns;
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                cost += s[i][j] != t[i][j];
        answer = std::min(answer, cost);
        s = rotate_90r(s);
    }
    verify_out << answer << '\n';
}
