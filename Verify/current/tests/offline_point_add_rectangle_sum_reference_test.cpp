// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/Offline_Point_Add_Rectangle_Sum.cpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <variant>
#include <vector>

namespace
{
using sirkit::PointAdd;
using sirkit::PointAddRectangleSumOperation;
using sirkit::RectangleSumRequest;

std::vector<std::int64_t> Scan(std::vector<PointAdd> points, const std::vector<PointAddRectangleSumOperation> &operations)
{
    std::vector<std::int64_t> result;
    for (const auto &operation : operations)
    {
        if (const auto *point = std::get_if<PointAdd>(&operation))
        {
            points.push_back(*point);
        }
        else
        {
            const auto &rectangle = std::get<RectangleSumRequest>(operation);
            std::uint64_t sum = 0;
            for (const auto &point : points)
            {
                if (rectangle.left <= point.x && point.x < rectangle.right &&
                    rectangle.bottom <= point.y && point.y < rectangle.top)
                {
                    sum += static_cast<std::uint64_t>(point.weight);
                }
            }
            result.push_back(std::bit_cast<std::int64_t>(sum));
        }
    }
    return result;
}
}

int main()
{
    using sirkit::offline_point_add_rectangle_sum;
    assert(offline_point_add_rectangle_sum({}, {}).empty());

    const std::vector<PointAddRectangleSumOperation> probes{RectangleSumRequest{-1, -1, 1, 1}};
    assert(offline_point_add_rectangle_sum({}, probes) == std::vector<std::int64_t>{0});
    assert(offline_point_add_rectangle_sum(std::vector<PointAdd>{{0, 0, 0}}, probes) == std::vector<std::int64_t>{0});
    const std::vector<PointAddRectangleSumOperation> empty_rectangles{
        RectangleSumRequest{0, 0, 0, 2},
        RectangleSumRequest{-1, 1, 1, 2},
    };
    assert((offline_point_add_rectangle_sum(std::vector<PointAdd>{{0, 0, 7}}, empty_rectangles) ==
            std::vector<std::int64_t>{0, 0}));
    assert(offline_point_add_rectangle_sum(std::vector<PointAdd>{{0, 0, 7}}, probes) == std::vector<std::int64_t>{7});

    constexpr auto kLow = std::numeric_limits<std::int64_t>::min();
    constexpr auto kHigh = std::numeric_limits<std::int64_t>::max();
    const std::vector<PointAdd> initial{
        {kLow, kLow, 3},
        {kLow, kLow, -1},
        {kHigh, kLow, 100},
        {0, kHigh, 101},
        {0, 0, kHigh},
    };
    const std::vector<PointAddRectangleSumOperation> operations{
        RectangleSumRequest{kLow, kLow, 0, 0},
        RectangleSumRequest{0, 0, 1, 1},
        PointAdd{0, 0, kHigh},
        PointAdd{0, 0, 2},
        RectangleSumRequest{0, 0, 1, 1},
        PointAdd{0, 0, kLow},
        RectangleSumRequest{0, 0, 1, 1},
        RectangleSumRequest{kLow, kLow, kHigh, kHigh},
    };
    const std::vector<std::int64_t> expected{2, kHigh, 0, kLow, kLow + 2};
    assert(offline_point_add_rectangle_sum(initial, operations) == expected);
    assert(Scan(initial, operations) == expected);

    std::mt19937_64 rng(626771);
    const auto coord = [&]
    {
        return static_cast<std::int64_t>(rng() % 17) - 8;
    };
    const auto point = [&]
    {
        return PointAdd{coord(), coord(), static_cast<std::int64_t>(rng() % 101) - 50};
    };
    for (int i = 0; i < 4400; ++i)
    {
        const auto n = rng() % 29;
        const auto q = rng() % 51;
        std::vector<PointAdd> points;
        std::vector<PointAddRectangleSumOperation> sequence;
        for (std::size_t j = 0; j < n; ++j)
        {
            points.push_back(point());
        }
        for (std::size_t j = 0; j < q; ++j)
        {
            if (rng() % 3 == 0)
            {
                sequence.emplace_back(point());
            }
            else
            {
                const auto [left, right] = std::minmax({coord(), coord()});
                const auto [bottom, top] = std::minmax({coord(), coord()});
                sequence.emplace_back(RectangleSumRequest{left, bottom, right, top});
            }
        }
        assert(offline_point_add_rectangle_sum(points, sequence) == Scan(points, sequence));
    }

    assert(sirkit::detail::CheckedPointAddRectangleSumEventCount(2, 2, 3, 10) == 10);
    constexpr std::array bad_counts{
        std::array{0, 0, 6},
        std::array{5, 0, 3},
        std::array{3, 2, 3},
    };
    for (const auto &[initial_count, update_count, query_count] : bad_counts)
    {
        try
        {
            (void)sirkit::detail::CheckedPointAddRectangleSumEventCount(initial_count, update_count, query_count, 10);
            assert(false);
        }
        catch (const std::length_error &)
        {
        }
    }

#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &rectangle : {RectangleSumRequest{2, 0, 1, 2}, RectangleSumRequest{0, 2, 2, 1}})
    {
        try
        {
            const std::vector<PointAddRectangleSumOperation> invalid{rectangle};
            (void)offline_point_add_rectangle_sum({}, invalid);
            assert(false);
        }
        catch (const std::invalid_argument &)
        {
        }
    }
#endif
}
