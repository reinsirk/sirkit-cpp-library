// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstdint>
int main() {
    int n{}; verify_in >> n;
    std::int64_t answer = 0;
    LineContainer2D points;
    for (int i = 0; i < n; ++i) {
        std::int64_t x{}, y{}; verify_in >> x >> y;
        points.add(x, y);
        answer = std::max({answer, points.max_rounded_ll(y, -x), -points.min_rounded_ll(y, -x)});
    }
    verify_out << answer << '\n';
}
