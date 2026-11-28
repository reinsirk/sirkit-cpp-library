#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_STATIC_CUBOID_SUM_DEFINED
#define SIRKIT_STATIC_CUBOID_SUM_DEFINED

struct StaticCuboidSumPoint
{
    std::int64_t x, y, z, weight;
};

struct StaticCuboidSumQuery
{
    std::array<std::int64_t, 3> lower;
    std::array<std::int64_t, 3> upper;
};

inline std::vector<std::int64_t> static_cuboid_sum(std::span<const StaticCuboidSumPoint> points,
                                                   std::span<const StaticCuboidSumQuery> queries)
{
    struct Boundary
    {
        std::int64_t x;
        std::size_t query;
        bool subtract;
    };

    std::vector<std::int64_t> answer(queries.size());
    std::vector<Boundary> boundaries;
    boundaries.reserve(detail::CheckedPointAddRectangleSumEventCount(0, 0, queries.size(), boundaries.max_size()));
    for (std::size_t i = 0; i < queries.size(); ++i)
    {
        const auto &[lower, upper] = queries[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (lower[0] > upper[0] || lower[1] > upper[1] || lower[2] > upper[2])
        {
            throw std::invalid_argument("static_cuboid_sum: reversed box");
        }
#endif
        if (lower[0] < upper[0] && lower[1] < upper[1] && lower[2] < upper[2])
        {
            boundaries.push_back({lower[0], i, true});
            boundaries.push_back({upper[0], i, false});
        }
    }
    if (boundaries.empty() || points.empty())
    {
        return answer;
    }

    auto ordered = std::vector(points.begin(), points.end());
    std::ranges::sort(ordered, {}, &StaticCuboidSumPoint::x);
    std::ranges::sort(boundaries, {}, &Boundary::x);
    std::vector<PointAddRectangleSumOperation> operations;
    operations.reserve(detail::CheckedPointAddRectangleSumEventCount(
        0, ordered.size(), boundaries.size() / 2, operations.max_size()));
    auto next_point = ordered.begin();
    for (const auto &boundary : boundaries)
    {
        // xが等しい点はprefixに加えない。半開区間ではその点は右側に含まれる。
        while (next_point != ordered.end() && next_point->x < boundary.x)
        {
            operations.emplace_back(PointAdd{next_point->y, next_point->z, next_point->weight});
            ++next_point;
        }
        const auto &[lower, upper] = queries[boundary.query];
        operations.emplace_back(RectangleSumRequest{lower[1], lower[2], upper[1], upper[2]});
    }
    const auto sums = offline_point_add_rectangle_sum({}, operations);
    for (std::size_t i = 0; i < boundaries.size(); ++i)
    {
        const auto &boundary = boundaries[i];
        auto weight = static_cast<std::uint64_t>(sums[i]);
        if (boundary.subtract)
        {
            weight = std::uint64_t{0} - weight;
        }
        auto &total = answer[boundary.query];
        total = static_cast<std::int64_t>(static_cast<std::uint64_t>(total) + weight);
    }
    return answer;
}

#endif
