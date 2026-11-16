// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <algorithm>
#include <cstdint>
#include <utility>
#include <vector>

int main() {
    int n{};
    verify_in >> n;
    std::vector<std::pair<std::int64_t, std::int64_t>> points(n);
    for (auto& [x, y] : points) verify_in >> x >> y;
    angle_sort(points);
    // ライブラリの[0,2π)順を、問題の(-π,π]順に合わせる。
    const auto split = std::find_if(points.begin(), points.end(), [](const auto& p) { return p.second < 0; });
    for (auto it = split; it != points.end(); ++it)
        verify_out << it->first << ' ' << it->second << '\n';
    for (auto it = points.begin(); it != split; ++it)
        verify_out << it->first << ' ' << it->second << '\n';
}
