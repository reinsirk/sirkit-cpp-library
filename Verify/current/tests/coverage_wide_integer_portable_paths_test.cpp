// SPDX-License-Identifier: CC0-1.0
#define SIRKIT_FRACTION_DISABLE_INT128
#include "DataStructure/Weighted_L1_Statistics.cpp"
#include "Geometry/Delaunay_Triangulation.cpp"
#include "Geometry/Integer_Geometry_3D.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <vector>

int main()
{
    sirkit::fenwick_tree<int> tree(std::vector{1, 2, 3});
    assert(tree.sum(0, 2) == 3);
    tree.add(1, 4);
    assert(tree.sum(1, 3) == 9);

    constexpr auto low = std::numeric_limits<std::int64_t>::min();
    constexpr auto high = std::numeric_limits<std::int64_t>::max();
    sirkit::WeightedL1Statistics statistics(std::array{low, high});
    statistics.insert(high);
    assert(statistics.absolute_deviation(low) == std::numeric_limits<std::uint64_t>::max());

    constexpr std::int64_t m = 1000000000;
    const sirkit::Point3i a{-m, -m, 0}, b{m, -m, 0}, c{-m, m, 0};
    const auto above = sirkit::signed_plane_distance(a, b, c, {0, 0, m});
    const auto below = sirkit::signed_plane_distance(a, b, c, {0, 0, -m});
    assert(above && std::abs(*above - 1.e9) < 1.e-6);
    assert(below && std::abs(*below + 1.e9) < 1.e-6);

    const std::array<sirkit::DelaunayPoint, 4> points{{{-m, -m}, {m, -m}, {m, m}, {-m, m - 1}}};
    const auto triangulation = sirkit::delaunay_triangulation(points);
    assert(triangulation.size() == 5);
    assert(std::ranges::find(triangulation, sirkit::DelaunayEdge{1, 3}) != triangulation.end());
}
