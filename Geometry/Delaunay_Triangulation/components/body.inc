#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

#ifndef SIRKIT_DELAUNAY_TRIANGULATION_DEFINED
#define SIRKIT_DELAUNAY_TRIANGULATION_DEFINED

using DelaunayPoint = std::pair<std::int64_t, std::int64_t>;
using DelaunayEdge = std::pair<std::size_t, std::size_t>;

namespace detail
{
struct DelaunayCircleTest
{
    bool operator()(const DelaunayPoint &x, const DelaunayPoint &y, const DelaunayPoint &z) const
    {
        FractionWide positive, negative;
        const auto add_term = [&](const DelaunayPoint &a, const DelaunayPoint &b, const DelaunayPoint &c)
        {
            const auto cross = EuclideanMstCross(b, c);
            const auto term = FractionWide::product(static_cast<std::uint64_t>(EuclideanMstNormSquared(a)),
                                                    fraction_magnitude(cross));
            auto &accumulator = cross < 0 ? negative : positive;
            accumulator = accumulator + term;
        };
        add_term(x, y, z);
        add_term(y, z, x);
        add_term(z, x, y);
        // 座標幅Wのとき、どちらの合計も12W^4以下で符号なし128ビットに収まる。
        return positive > negative;
    }
};
} // namespace detail

[[nodiscard]] inline std::vector<DelaunayEdge> delaunay_triangulation(std::span<const DelaunayPoint> points)
{
    if (points.size() < 2)
    {
        return {};
    }
    std::vector<std::size_t> order(points.size());
    std::iota(order.begin(), order.end(), std::size_t{0});
    std::ranges::sort(order, {}, [&](std::size_t i)
                      {
                          return std::pair(points[i], i);
                      });

    const auto min_x = points[order.front()].first;
    const auto min_y = std::ranges::min_element(points, {}, &DelaunayPoint::second)->second;
    const auto translate = [](std::int64_t coordinate, std::int64_t minimum)
    {
        constexpr std::uint64_t kMaxSpan = 2'000'000'000;
        const auto width = static_cast<std::uint64_t>(coordinate) - static_cast<std::uint64_t>(minimum);
        if (width > kMaxSpan)
        {
            throw std::overflow_error("delaunay_triangulation: coordinate range is too wide");
        }
        return static_cast<std::int64_t>(width);
    };
    std::vector<DelaunayPoint> unique;
    unique.reserve(order.size());
    std::size_t distinct = 0;
    for (const auto i : order)
    {
        if (distinct == 0 || points[i] != points[order[distinct - 1]])
        {
            const auto [x, y] = points[i];
            unique.emplace_back(translate(x, min_x), translate(y, min_y));
            order[distinct++] = i;
        }
    }
    if (distinct < 2)
    {
        return {};
    }

    const auto candidates = detail::EuclideanMstTriangulation<detail::DelaunayCircleTest>(unique).WeightedEdges();
    std::vector<DelaunayEdge> result;
    result.reserve(candidates.size());
    for (const auto &edge : candidates)
    {
        const auto [a, b] = std::minmax(order[edge.from], order[edge.to]);
        result.emplace_back(a, b);
    }
    return result;
}

#endif
