// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/geometory.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

int main() {
    using sirkit::geo::PointI;
    std::mt19937 generator(0x184D0);
    for (const std::int64_t width : {255, 200'000'000}) {
        const std::int64_t low = -width / 2;
        const std::int64_t high = low + width;
        const std::vector<PointI> corners{{low, low}, {high, low}, {high, high}, {low, high}};
        const auto coordinate = [&] { return low + static_cast<std::int64_t>(generator()) % (width + 1); };
        for (const std::size_t count : {2047, 2048, 16383, 16384}) {
            auto points = corners;
            while (points.size() < count) {
                points.push_back({coordinate(), coordinate()});
            }
            points.back() = corners.front();
            std::shuffle(points.begin(), points.end(), generator);
            // 全点が四隅で作る矩形内にあり、四隅も入力に含まれる。
            assert(sirkit::geo::convex_hull(points) == corners);
            std::sort(points.begin(), points.end());
            assert(sirkit::convex_hull_sorted(points) == corners);
        }
    }
}
