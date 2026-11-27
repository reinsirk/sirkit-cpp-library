#include <cstdint>
#include <span>
#include <utility>

#ifndef SIRKIT_ALL_FURTHEST_NEIGHBORS_DEFINED
#define SIRKIT_ALL_FURTHEST_NEIGHBORS_DEFINED

namespace reinsirk::detail
{
using FurthestNeighborPoint = std::pair<std::int64_t, std::int64_t>;

inline int FurthestNeighborHeight(std::size_t count)
{
    constexpr auto kMaxRows = static_cast<std::size_t>(std::numeric_limits<int>::max() / 2) + 1;
    if (count > kMaxRows)
    {
        throw std::length_error("all_furthest_neighbors: too many vertices");
    }
    return static_cast<int>(count);
}

inline void CheckFurthestNeighborBounds(std::span<const FurthestNeighborPoint> vertices)
{
    const auto [min_x, max_x] = std::ranges::minmax_element(vertices, {}, &FurthestNeighborPoint::first);
    const auto [min_y, max_y] = std::ranges::minmax_element(vertices, {}, &FurthestNeighborPoint::second);
    const auto width = [](std::int64_t low, std::int64_t high)
    {
        return static_cast<std::uint64_t>(high) - static_cast<std::uint64_t>(low);
    };
    constexpr std::uint64_t kMaxSpan = 2'000'000'000;
    if (width(min_x->first, max_x->first) > kMaxSpan || width(min_y->second, max_y->second) > kMaxSpan)
    {
        throw std::overflow_error("all_furthest_neighbors: coordinate range is too wide");
    }
}

#ifndef SKIP_RUNTIME_CHECKS
inline void CheckStrictFurthestNeighborPolygon(std::span<const FurthestNeighborPoint> vertices)
{
    const auto edge = [](FurthestNeighborPoint a, FurthestNeighborPoint b)
    {
        return FurthestNeighborPoint{b.first - a.first, b.second - a.second};
    };
    const auto cross = [](FurthestNeighborPoint a, FurthestNeighborPoint b)
    {
        return a.first * b.second - a.second * b.first;
    };
    const bool clockwise = cross(edge(vertices[0], vertices[1]), edge(vertices[1], vertices[2])) < 0;
    const auto direction = [&](FurthestNeighborPoint a, FurthestNeighborPoint b)
    {
        auto delta = edge(a, b);
        if (clockwise)
        {
            delta.second = -delta.second;
        }
        return delta;
    };
    const auto lower_half = [](FurthestNeighborPoint p)
    {
        return p.second < 0 || (p.second == 0 && p.first < 0);
    };
    auto previous = direction(vertices.back(), vertices.front());
    std::size_t wraps = 0;
    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        const auto next = i + 1 == vertices.size() ? 0 : i + 1;
        const auto current = direction(vertices[i], vertices[next]);
        if (cross(previous, current) <= 0)
        {
            throw std::invalid_argument("all_furthest_neighbors: expected a strictly convex polygon");
        }
        wraps += lower_half(previous) && !lower_half(current);
        previous = current;
    }
    // すべての転角が正でも星形の巡回はあり得る。方向がちょうど1周することを確認する。
    if (wraps != 1)
    {
        throw std::invalid_argument("all_furthest_neighbors: expected one boundary traversal");
    }
}
#endif
} // namespace reinsirk::detail

inline std::vector<int> all_furthest_neighbors(std::span<const std::pair<std::int64_t, std::int64_t>> vertices)
{
    if (vertices.empty())
    {
        return {};
    }
    const int n = reinsirk::detail::FurthestNeighborHeight(vertices.size());
    reinsirk::detail::CheckFurthestNeighborBounds(vertices);
    if (n == 1)
    {
        return {0};
    }
    if (n == 2)
    {
        return {1, 0};
    }
#ifndef SKIP_RUNTIME_CHECKS
    reinsirk::detail::CheckStrictFurthestNeighborPolygon(vertices);
#endif

    const auto matrix = [&](int row, int column) -> std::int64_t
    {
        // 循環順に列を伸ばし、元の1周に含まれない列を補完して全単調にする。
        if (column <= row)
        {
            return column - row;
        }
        if (column - row >= n)
        {
            return -1;
        }
        const auto &a = vertices[row];
        const auto &b = vertices[column < n ? column : column - n];
        const auto dx = a.first - b.first, dy = a.second - b.second;
        return dx * dx + dy * dy;
    };
    auto result = smawk(n, n + (n - 1), [&](int row, int before, int candidate)
                        {
                            return matrix(row, before) < matrix(row, candidate);
                        });
    for (auto &index : result)
    {
        index %= n;
    }
    return result;
}
#endif
