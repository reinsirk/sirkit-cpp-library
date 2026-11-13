// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

int main() {
    int n = 0;
    verify_in >> n;
    std::vector<std::pair<Fraction, int>> people;
    people.reserve(n);
    for (int i = 1; i <= n; ++i) {
        std::int64_t a = 0, b = 0;
        verify_in >> a >> b;
        people.emplace_back(Fraction(a, a + b), i);
    }
    std::sort(people.begin(), people.end(), [](const auto& a, const auto& b) {
        if (a.first == b.first) return a.second < b.second;
        return a.first > b.first;
    });
    for (int i = 0; i < n; ++i) {
        verify_out << people[i].second << (i + 1 == n ? '\n' : ' ');
    }
}
