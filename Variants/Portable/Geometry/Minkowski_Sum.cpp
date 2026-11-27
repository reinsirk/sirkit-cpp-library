// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Geometry/Minkowski_Sum.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/minkowski_sum.md
//
// API一覧
// minkowski_sum(left, right): 整数座標の凸多角形2つのミンコフスキー和の頂点を返す。O(N+M)時間・領域。
// 頂点は境界順（時計回り・反時計回り）。点、線分、連続する重複点、辺上の中間点も扱う。
// 戻り値は(y, x)が最小の頂点から反時計回り。始点の再掲と辺上の中間点を除く。
// 一方が空なら空を返す。各軸の座標幅がuint32_t、和の座標がint64_tを超える場合はoverflow_error。
// 通常版では、一直線上の入力を除き凸多角形の境界順でない場合にinvalid_argument。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MINKOWSKI_SUM_DEFINED
#define SIRKIT_MINKOWSKI_SUM_DEFINED

namespace detail
{
using MinkowskiPoint = std::pair<std::int64_t, std::int64_t>;

inline bool MinkowskiPointLess(const MinkowskiPoint &a, const MinkowskiPoint &b)
{
    return std::pair(a.second, a.first) < std::pair(b.second, b.first);
}

inline MinkowskiPoint MinkowskiDirection(const MinkowskiPoint &from, const MinkowskiPoint &to)
{
    return {to.first - from.first, to.second - from.second};
}

inline MinkowskiPoint MinkowskiEdge(const std::vector<MinkowskiPoint> &polygon, std::size_t i)
{
    const auto next = i + 1 == polygon.size() ? 0 : i + 1;
    return MinkowskiDirection(polygon[i], polygon[next]);
}

inline std::uint64_t MinkowskiMagnitude(std::int64_t value)
{
    return static_cast<std::uint64_t>(value < 0 ? -value : value);
}

inline int MinkowskiCrossSign(const MinkowskiPoint &a, const MinkowskiPoint &b)
{
    // 座標差はuint32_tの大きさに収まる。積の差を取らず、符号と絶対値で比較する。
    const auto left = MinkowskiMagnitude(a.first) * MinkowskiMagnitude(b.second);
    const auto right = MinkowskiMagnitude(a.second) * MinkowskiMagnitude(b.first);
    const bool left_negative = left != 0 && ((a.first < 0) != (b.second < 0));
    const bool right_negative = right != 0 && ((a.second < 0) != (b.first < 0));
    if (left_negative != right_negative)
    {
        return left_negative ? -1 : 1;
    }
    const int order = (left > right) - (left < right);
    return left_negative ? -order : order;
}

inline bool MinkowskiLowerHalf(const MinkowskiPoint &direction)
{
    return direction.second < 0 || (direction.second == 0 && direction.first < 0);
}

inline int MinkowskiAngleOrder(const MinkowskiPoint &a, const MinkowskiPoint &b)
{
    const bool a_lower = MinkowskiLowerHalf(a);
    const bool b_lower = MinkowskiLowerHalf(b);
    if (a_lower != b_lower)
    {
        return a_lower ? 1 : -1;
    }
    return -MinkowskiCrossSign(a, b);
}

struct MinkowskiBounds
{
    std::int64_t min_x;
    std::int64_t max_x;
    std::int64_t min_y;
    std::int64_t max_y;
};

inline MinkowskiBounds MinkowskiGetBounds(std::span<const MinkowskiPoint> polygon)
{
    auto [min_x, max_x] = std::ranges::minmax_element(polygon, {}, &MinkowskiPoint::first);
    auto [min_y, max_y] = std::ranges::minmax_element(polygon, {}, &MinkowskiPoint::second);
    const auto check = [](std::int64_t low, std::int64_t high)
    {
        if (static_cast<std::uint64_t>(high) - static_cast<std::uint64_t>(low) >
            std::numeric_limits<std::uint32_t>::max())
        {
            throw std::overflow_error("minkowski_sum: input coordinate range is too wide");
        }
    };
    check(min_x->first, max_x->first);
    check(min_y->second, max_y->second);
    return {min_x->first, max_x->first, min_y->second, max_y->second};
}

inline void CheckMinkowskiCoordinateSum(std::int64_t a, std::int64_t b)
{
    if ((b > 0 && a > std::numeric_limits<std::int64_t>::max() - b) ||
        (b < 0 && a < std::numeric_limits<std::int64_t>::min() - b))
    {
        throw std::overflow_error("minkowski_sum: sum coordinate exceeds int64_t");
    }
}

inline std::vector<MinkowskiPoint> NormalizeMinkowskiPolygon(std::span<const MinkowskiPoint> vertices)
{
    std::vector<MinkowskiPoint> polygon;
    polygon.reserve(vertices.size());
    for (const auto &point : vertices)
    {
        if (polygon.empty() || polygon.back() != point)
        {
            polygon.push_back(point);
        }
    }
    if (polygon.size() > 1 && polygon.front() == polygon.back())
    {
        polygon.pop_back();
    }
    if (polygon.size() == 1)
    {
        return polygon;
    }

    auto [first, last] = std::ranges::minmax_element(polygon, MinkowskiPointLess);
    const auto i = static_cast<std::size_t>(first - polygon.begin());
    const auto previous = i == 0 ? polygon.size() - 1 : i - 1;
    const int orientation = MinkowskiCrossSign(MinkowskiEdge(polygon, previous), MinkowskiEdge(polygon, i));
    if (orientation == 0)
    {
#ifndef SKIP_RUNTIME_CHECKS
        const auto axis = MinkowskiDirection(*first, *last);
        for (const auto &point : polygon)
        {
            if (MinkowskiCrossSign(axis, MinkowskiDirection(*first, point)) != 0)
            {
                throw std::invalid_argument("minkowski_sum: expected convex boundary order");
            }
        }
#endif
        return {*first, *last};
    }

    std::ranges::rotate(polygon, first);
    if (orientation < 0)
    {
        std::reverse(polygon.begin() + 1, polygon.end());
    }
#ifndef SKIP_RUNTIME_CHECKS
    for (std::size_t edge = 1; edge < polygon.size(); ++edge)
    {
        if (MinkowskiAngleOrder(MinkowskiEdge(polygon, edge - 1), MinkowskiEdge(polygon, edge)) > 0)
        {
            throw std::invalid_argument("minkowski_sum: expected convex boundary order");
        }
    }
#endif
    return polygon;
}
} // namespace detail

inline std::vector<std::pair<std::int64_t, std::int64_t>> minkowski_sum(
    std::span<const std::pair<std::int64_t, std::int64_t>> left,
    std::span<const std::pair<std::int64_t, std::int64_t>> right)
{
    if (left.empty() || right.empty())
    {
        return {};
    }
    const auto a_bounds = detail::MinkowskiGetBounds(left);
    const auto b_bounds = detail::MinkowskiGetBounds(right);
    detail::CheckMinkowskiCoordinateSum(a_bounds.min_x, b_bounds.min_x);
    detail::CheckMinkowskiCoordinateSum(a_bounds.max_x, b_bounds.max_x);
    detail::CheckMinkowskiCoordinateSum(a_bounds.min_y, b_bounds.min_y);
    detail::CheckMinkowskiCoordinateSum(a_bounds.max_y, b_bounds.max_y);
    const auto a = detail::NormalizeMinkowskiPolygon(left);
    const auto b = detail::NormalizeMinkowskiPolygon(right);
    const auto a_edge_count = a.size() == 1 ? 0 : a.size();
    const auto b_edge_count = b.size() == 1 ? 0 : b.size();

    std::vector<detail::MinkowskiPoint> result;
    const auto append = [&](std::size_t i, std::size_t j)
    {
        const auto &p = a[i == a.size() ? 0 : i];
        const auto &q = b[j == b.size() ? 0 : j];
        result.emplace_back(p.first + q.first, p.second + q.second);
    };
    const auto advance = [](const auto &polygon, std::size_t &i, std::size_t count, const auto &direction)
    {
        while (i < count && detail::MinkowskiAngleOrder(detail::MinkowskiEdge(polygon, i), direction) == 0)
        {
            ++i;
        }
    };
    std::size_t i = 0;
    std::size_t j = 0;
    append(i, j);
    while (i < a_edge_count || j < b_edge_count)
    {
        // 同じ方向へ進む両方の辺をまとめ、直線上の中間点を飛ばす。
        const bool use_a = j == b_edge_count ||
                           (i < a_edge_count && detail::MinkowskiAngleOrder(
                                                    detail::MinkowskiEdge(a, i), detail::MinkowskiEdge(b, j)) <= 0);
        const auto direction = use_a ? detail::MinkowskiEdge(a, i) : detail::MinkowskiEdge(b, j);
        advance(a, i, a_edge_count, direction);
        advance(b, j, b_edge_count, direction);
        if (i < a_edge_count || j < b_edge_count)
        {
            append(i, j);
        }
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
