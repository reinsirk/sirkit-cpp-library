// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cmath>
#include <cstdint>
#include <vector>

// 非減少の1次元列をy=0へ置く。最近点対のdistance_squaredを実出力へ反映。
// 整列済みAPIでもO(N log N)時間・O(N)領域。二乗距離は完全平方数。
int main() {
    int n{}; verify_in >> n;
    std::vector<sirkit::P> points(n);
    for (auto& point : points) verify_in >> point.first;
    const auto answer=sirkit::nearest_two_points_sorted(points);
    const auto distance=static_cast<std::int64_t>(std::sqrt(static_cast<long double>(answer->distance_squared)));
    verify_out << distance << '\n' << static_cast<std::int64_t>(points.back().first)-points.front().first << '\n';
}
