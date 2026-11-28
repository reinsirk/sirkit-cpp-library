// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Trapezoid_Count.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/trapezoid_count.md
//
// API一覧
// TrapezoidPoint = pair<int32_t,int32_t>。
// count_trapezoids(points): 4点の選び方のうち面積が正の台形と平行四辺形の個数を返す。
// 結果のtrapezoidsは平行四辺形を含む。点は相異なり、3点以上の共線は可。n<=92682。
// O(n^2 log(n+1))時間、O(n^2+1)領域。点の重複と個数超過はチェック有効時に例外。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TRAPEZOID_COUNT_DEFINED
#define SIRKIT_TRAPEZOID_COUNT_DEFINED

using TrapezoidPoint = std::pair<std::int32_t, std::int32_t>;

struct TrapezoidCounts
{
    std::uint64_t trapezoids, parallelograms;
};

namespace detail
{
struct TrapezoidSegment
{
    std::array<std::int64_t, 2> direction, doubled_midpoint;
    std::int64_t line;
};

template <class Major, class Minor>
std::uint64_t CountTrapezoidPairs(std::vector<TrapezoidSegment> &segments, Major major, Minor minor)
{
    std::sort(segments.begin(), segments.end(),
              [&](const auto &a, const auto &b)
              {
                  return std::tie(major(a), minor(a)) < std::tie(major(b), minor(b));
              });
    std::uint64_t result = 0;
    for (std::size_t first = 0; first < segments.size();)
    {
        const auto group_begin = first;
        do
        {
            auto end = first + 1;
            while (end < segments.size() && major(segments[first]) == major(segments[end]) &&
                   minor(segments[first]) == minor(segments[end]))
            {
                ++end;
            }
            result += static_cast<std::uint64_t>(first - group_begin) * (end - first);
            first = end;
        } while (first < segments.size() && major(segments[group_begin]) == major(segments[first]));
    }
    return result;
}
}

[[nodiscard]] inline TrapezoidCounts count_trapezoids(std::span<const TrapezoidPoint> points)
{
    std::vector<TrapezoidPoint> vertices(points.begin(), points.end());
    std::sort(vertices.begin(), vertices.end());
    if (vertices.size() < 4)
    {
        return {};
    }

    std::vector<detail::TrapezoidSegment> segments;
    segments.reserve(vertices.size() * (vertices.size() - 1) / 2);
    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        const auto [x, y] = vertices[i];
        for (std::size_t j = i + 1; j < vertices.size(); ++j)
        {
            const auto [u, v] = vertices[j];
            const auto dx = static_cast<std::int64_t>(u) - x;
            const auto dy = static_cast<std::int64_t>(v) - y;
            const auto step = std::gcd(dx, dy);
            // int32_tどうしの積の差は絶対値が2^63未満。
            const auto cross = static_cast<std::int64_t>(x) * v - static_cast<std::int64_t>(y) * u;
            segments.push_back({{dx / step, dy / step},
                                {static_cast<std::int64_t>(x) + u, static_cast<std::int64_t>(y) + v},
                                cross / step});
        }
    }
    const auto direction = [](const auto &segment) -> const auto &
    {
        return segment.direction;
    };
    const auto parallel_sides = detail::CountTrapezoidPairs(segments, direction,
                                                            [](const auto &segment) -> const auto &
                                                            {
                                                                return segment.line;
                                                            });
    const auto parallelograms = detail::CountTrapezoidPairs(segments, [](const auto &segment) -> const auto &
                                                            {
                                                                return segment.doubled_midpoint;
                                                            },
                                                            direction);
    return {parallel_sides - parallelograms, parallelograms};
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
