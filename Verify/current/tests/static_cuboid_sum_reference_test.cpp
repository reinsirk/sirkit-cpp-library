// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/Static_Cuboid_Sum.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Point = sirkit::StaticCuboidSumPoint;
using Query = sirkit::StaticCuboidSumQuery;

std::int64_t direct(const std::vector<Point> &points, const Query &query)
{
    std::uint64_t result = 0;
    for (const auto &point : points)
    {
        const std::array coordinates{point.x, point.y, point.z};
        bool inside = true;
        for (int d = 0; d < 3; ++d)
        {
            inside &= query.lower[d] <= coordinates[d] && coordinates[d] < query.upper[d];
        }
        if (inside)
        {
            result += static_cast<std::uint64_t>(point.weight);
        }
    }
    return static_cast<std::int64_t>(result);
}

void check(const std::vector<Point> &points, const std::vector<Query> &queries)
{
    const auto actual = sirkit::static_cuboid_sum(points, queries);
    assert(actual.size() == queries.size());
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        assert(actual[i] == direct(points, queries[i]));
    }
}
}

int main()
{
    constexpr auto kLo = std::numeric_limits<std::int64_t>::min();
    constexpr auto kHi = std::numeric_limits<std::int64_t>::max();
    check({}, {});
    check({}, {{{-3, 4, -3}, {1, 5, 3}}});
    check({{1, 3, -5, -3}}, {{{0, 2, -5}, {1, 2, 5}}});
    check({{kLo, -1, kLo, kHi}, {kLo, -1, kLo, kHi}, {kHi, 2, -1, 800}, {kLo + 1, -1, kLo + 1, 20}, {0, 0, 0, kLo}},
          {{{kLo, kLo, kLo}, {kHi, kHi, kHi}}, {{kLo, -1, kLo}, {kLo + 1, 0, kLo + 1}}, {{kLo + 1, -2, kLo}, {kLo + 2, 20, kHi}}, {{0, 0, 0}, {1, 1, 1}}});
#ifndef SKIP_RUNTIME_CHECKS
    for (int d = 0; d < 3; ++d)
    {
        Query query{{1, 1, 1}, {3, 3, 3}};
        query.upper[d] = 0;
        bool caught = false;
        try
        {
            const std::array invalid{query};
            static_cast<void>(sirkit::static_cuboid_sum({}, invalid));
        }
        catch (const std::invalid_argument &)
        {
            caught = true;
        }
        assert(caught);
    }
#endif
    std::mt19937 random(51721);
    const auto draw = [&](int radius) -> std::int64_t
    {
        return static_cast<int>(random() % (2 * radius + 1)) - radius;
    };
    for (int trial = 0; trial < 650; ++trial)
    {
        std::vector<Point> points(1 + random() % 36);
        for (auto &point : points)
        {
            point = {draw(8), draw(8), draw(8), draw(10)};
        }
        std::vector<Query> queries(1 + random() % 31);
        for (auto &query : queries)
        {
            for (int d = 0; d < 3; ++d)
            {
                const auto a = draw(9), b = draw(9);
                query.lower[d] = std::min(a, b);
                query.upper[d] = std::max(a, b);
            }
        }
        check(points, queries);
    }
}
