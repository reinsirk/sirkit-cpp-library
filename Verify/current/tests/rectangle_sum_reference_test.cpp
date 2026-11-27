// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_RECTANGLE_SUM_TEST_SOURCE
#define SIRKIT_RECTANGLE_SUM_TEST_SOURCE "../../../Geometry/Rectangle_Sum.cpp"
#endif
#include SIRKIT_RECTANGLE_SUM_TEST_SOURCE
#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
using Point = sirkit::RectangleSumPoint;
using Query = sirkit::RectangleSumQuery;

std::vector<std::int64_t> Direct(std::span<const Point> points, std::span<const Query> queries)
{
    std::vector<std::int64_t> sums;
    for (const auto &query : queries)
    {
        std::int64_t sum = 0;
        for (const auto &[x, y, weight] : points)
        {
            if (query.left <= x && x < query.right && query.bottom <= y && y < query.top)
            {
                sum += weight;
            }
        }
        sums.push_back(sum);
    }
    return sums;
}

template <class Exception, class Operation>
bool Throws(Operation operation)
{
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        return true;
    }
    return false;
}
} // namespace

int main()
{
    assert(sirkit::rectangle_sum({}, {}).empty());
    const std::array queries{Query{0, 0, 1, 1}, Query{0, 1, 1, 2}, Query{1, 0, 1, 1}, Query{0, 1, 1, 1}};
    assert(sirkit::rectangle_sum({}, queries) == std::vector<std::int64_t>(queries.size()));

    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    const std::array extreme_points{Point{kMin, kMin, 3}, Point{kMin, kMin, -5}, Point{kMax, kMin, 17},
                                    Point{kMin, kMax, 19}, Point{kMax - 1, kMax - 1, 7}, Point{-1, -1, -4}};
    const std::array extreme_queries{Query{kMin, kMin, kMax, kMax}, Query{kMin, kMin, kMin + 1, kMin + 1},
                                     Query{kMax - 1, kMax - 1, kMax, kMax}, Query{-1, -1, 0, 0},
                                     Query{0, -1, 1, 0}, Query{-1, 0, 0, 1}, Query{kMax, 0, kMax, 1}};
    assert((sirkit::rectangle_sum(extreme_points, extreme_queries) ==
            std::vector<std::int64_t>{1, -2, 7, -4, 0, 0, 0}));

    const std::array wide_weights{Point{0, 0, kMax}, Point{1, 0, kMax}, Point{2, 0, kMin}, Point{3, 0, -kMax}};
    const std::array weight_queries{Query{0, 0, 4, 1}, Query{0, 0, 2, 1}, Query{1, 0, 3, 1}, Query{2, 0, 3, 1},
                                    Query{1, 0, 2, 1}, Query{9, 0, 10, 1}};
    assert((sirkit::rectangle_sum(wide_weights, weight_queries) ==
            std::vector<std::int64_t>{-1, -2, -1, kMin, kMax, 0}));

    std::mt19937_64 random(172601);
    for (int i = 0; i < 500; ++i)
    {
        const auto n = random() % 47, q = random() % 53;
        const auto coordinate = [&random]()
        {
            return static_cast<std::int64_t>(random() % 23) - 11;
        };
        std::vector<Point> points;
        std::vector<Query> rectangles;
        for (std::size_t j = 0; j < n; ++j)
        {
            const auto weight = static_cast<std::int64_t>(random() % 2'000'000'001) - 1'000'000'000;
            points.push_back({coordinate(), coordinate(), weight});
        }
        for (std::size_t j = 0; j < q; ++j)
        {
            const auto a = coordinate(), b = coordinate(), c = coordinate(), d = coordinate();
            rectangles.push_back({std::min(a, c), std::min(b, d), std::max(a, c), std::max(b, d)});
        }
        assert(sirkit::rectangle_sum(points, rectangles) == Direct(points, rectangles));
    }

#ifndef SKIP_RUNTIME_CHECKS
    assert(Throws<std::invalid_argument>([]
                                         {
                                             sirkit::rectangle_sum({}, std::array{Query{2, 0, 1, 0}});
                                         }));
    assert(Throws<std::invalid_argument>([]
                                         {
                                             sirkit::rectangle_sum({}, std::array{Query{0, 2, 1, 1}});
                                         }));
#endif
    assert(sirkit::detail::CheckedRectangleSumEventCount(3, 7) == 6);
    assert(Throws<std::length_error>([]
                                     {
                                         const auto capacity = std::numeric_limits<std::size_t>::max();
                                         sirkit::detail::CheckedRectangleSumEventCount(capacity / 2 + 1, capacity);
                                     }));
}
