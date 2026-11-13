// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

int main() {
    int tests; verify_in >> tests;
    while (tests--) {
        int n; verify_in >> n;
        std::vector<P> points(n);
        for (auto& [x, y] : points) verify_in >> x >> y;
        // この問題はn>=2。距離0の対があれば、それより小さい距離はない。
        if (points[0] == points[1]) {
            verify_out << "0 1\n";
            continue;
        }
        const auto answer = nearest_two_points(points);
        verify_out << answer->first_index << ' ' << answer->second_index << '\n';
    }
}
