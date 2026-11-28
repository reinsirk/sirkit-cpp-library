// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Half_Plane_Intersection.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/half_plane_intersection.md
//
// API一覧
// HalfPlane{a,b,c}: ax+by<=c。|a|,|b|<=2*10^9、|c|<=2*10^18。
// half_plane_intersection(planes, lower, upper): 指定矩形内の共通部分。反時計回りの頂点(doubleの対)。
// 凸多角形でない共通部分も、線分は2点、点は1点、空は空列で返す。境界を含む。
// convex_polygon_intersection(a,b): 2つの凸多角形の共通部分を求める。入力は正の面積・反時計回り。
// 矩形の端点と入力頂点はpair<int64_t,int64_t>で、各座標の絶対値は10^9以下。
// 半平面数hに対しO(h log(h+1))時間・O(h+1)領域。半平面の判定は整数で正確、返す座標は丸める。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <deque>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HALF_PLANE_INTERSECTION_DEFINED
#define SIRKIT_HALF_PLANE_INTERSECTION_DEFINED

struct HalfPlane
{
    std::int64_t a;
    std::int64_t b;
    std::int64_t c;
};

namespace detail
{
using HalfPlaneVertex = std::pair<std::int64_t, std::int64_t>;
using HalfPlaneRealVertex = std::pair<double, double>;

struct HalfPlaneInteger
{
    std::uint64_t low = 0;
    std::uint64_t high = 0;

    HalfPlaneInteger operator+(const HalfPlaneInteger &other) const noexcept
    {
        const auto sum = low + other.low;
        return {sum, high + other.high + static_cast<unsigned>(sum < low)};
    }

    HalfPlaneInteger operator-(const HalfPlaneInteger &other) const noexcept
    {
        return {low - other.low, high - other.high - static_cast<unsigned>(low < other.low)};
    }

    static HalfPlaneInteger Product(std::int64_t a, std::int64_t b) noexcept
    {
        const auto x = Magnitude(a);
        const auto y = Magnitude(b);
        constexpr std::uint64_t kMask = 0xffffffff;
        const auto x0 = x & kMask, x1 = x >> 32;
        const auto y0 = y & kMask, y1 = y >> 32;
        const auto low_product = x0 * y0;
        const auto mixed_x = x1 * y0 + (low_product >> 32);
        const auto mixed_y = x0 * y1 + (mixed_x & kMask);
        const HalfPlaneInteger result{(mixed_y << 32) | (low_product & kMask),
                                      x1 * y1 + (mixed_x >> 32) + (mixed_y >> 32)};
        return (a < 0) == (b < 0) ? result : HalfPlaneInteger{} - result;
    }

    bool IsZero() const noexcept
    {
        return (low | high) == 0;
    }

    bool IsNegative() const noexcept
    {
        return (high >> 63) != 0;
    }

    double ToDouble() const noexcept
    {
        const auto magnitude = IsNegative() ? HalfPlaneInteger{} - *this : *this;
        const double value = std::ldexp(static_cast<double>(magnitude.high), 64) +
                             static_cast<double>(magnitude.low);
        return IsNegative() ? -value : value;
    }

private:
    static std::uint64_t Magnitude(std::int64_t value) noexcept
    {
        const auto unsigned_value = static_cast<std::uint64_t>(value);
        return value < 0 ? std::uint64_t{0} - unsigned_value : unsigned_value;
    }
};

inline constexpr std::int64_t kHalfPlanePointLimit = 1000000000;
inline constexpr std::int64_t kHalfPlaneNormalLimit = 2 * kHalfPlanePointLimit;
inline constexpr std::int64_t kHalfPlaneOffsetLimit = kHalfPlaneNormalLimit * kHalfPlanePointLimit;

inline bool half_plane_within(std::int64_t value, std::int64_t limit) noexcept
{
    return -limit <= value && value <= limit;
}

inline void check_half_plane_vertex(const HalfPlaneVertex &point)
{
    if (!half_plane_within(point.first, kHalfPlanePointLimit) ||
        !half_plane_within(point.second, kHalfPlanePointLimit))
    {
        throw std::out_of_range("half_plane_intersection: coordinate magnitude");
    }
}

inline std::int64_t half_plane_cross(const HalfPlane &a, const HalfPlane &b) noexcept
{
    return a.a * b.b - a.b * b.a;
}

inline bool half_plane_first_half(const HalfPlane &line) noexcept
{
    return line.b > 0 || (line.b == 0 && line.a > 0);
}

inline std::int64_t half_plane_scale(const HalfPlane &line) noexcept
{
    return std::abs(line.a != 0 ? line.a : line.b);
}

inline bool half_plane_sort(const HalfPlane &a, const HalfPlane &b) noexcept
{
    const bool first_a = half_plane_first_half(a);
    const bool first_b = half_plane_first_half(b);
    if (first_a != first_b)
    {
        return first_a;
    }
    const std::int64_t cross = half_plane_cross(a, b);
    if (cross != 0)
    {
        return cross > 0;
    }
    // 同じ向きでは原点からの符号付き距離が小さい制約を先にする。
    return (HalfPlaneInteger::Product(a.c, half_plane_scale(b)) -
            HalfPlaneInteger::Product(b.c, half_plane_scale(a)))
        .IsNegative();
}

inline HalfPlaneInteger half_plane_side_at_vertex(const HalfPlane &a, const HalfPlane &b, const HalfPlane &query) noexcept
{
    // 交点を代入した値の分子。分母はcross(a,b)。
    return HalfPlaneInteger::Product(a.c, half_plane_cross(query, b)) +
           HalfPlaneInteger::Product(b.c, half_plane_cross(a, query)) -
           HalfPlaneInteger::Product(query.c, half_plane_cross(a, b));
}

inline bool half_plane_outside(const HalfPlane &a, const HalfPlane &b, const HalfPlane &test) noexcept
{
    const auto side = half_plane_side_at_vertex(a, b, test);
    return !side.IsZero() && (side.IsNegative() == (half_plane_cross(a, b) < 0));
}

inline HalfPlaneRealVertex half_plane_vertex(const HalfPlane &a, const HalfPlane &b) noexcept
{
    const auto x = HalfPlaneInteger::Product(a.c, b.b) - HalfPlaneInteger::Product(a.b, b.c);
    const auto y = HalfPlaneInteger::Product(a.a, b.c) - HalfPlaneInteger::Product(a.c, b.a);
    const auto determinant = static_cast<double>(half_plane_cross(a, b));
    return {x.ToDouble() / determinant, y.ToDouble() / determinant};
}

inline void half_plane_drop_front(std::deque<HalfPlane> &active, const HalfPlane &limit, std::size_t minimum_size)
{
    while (active.size() >= minimum_size && half_plane_outside(active[0], active[1], limit))
    {
        active.pop_front();
    }
}

inline std::vector<HalfPlaneRealVertex> intersect_bounded_half_planes(std::vector<HalfPlane> lines)
{
    std::sort(lines.begin(), lines.end(), half_plane_sort);
    const auto same_direction = [](const auto &a, const auto &b)
    {
        return half_plane_first_half(a) == half_plane_first_half(b) &&
               half_plane_cross(a, b) == 0;
    };
    lines.erase(std::unique(lines.begin(), lines.end(), same_direction), lines.end());

    std::deque<HalfPlane> active;
    for (const auto &line : lines)
    {
        while (active.size() >= 2 && half_plane_outside(active[active.size() - 2], active.back(), line))
        {
            active.pop_back();
        }
        half_plane_drop_front(active, line, 2);
        if (!active.empty() && half_plane_cross(active.back(), line) == 0)
        {
            // 矩形制約を入れているので、対向する直線が隣接するのは境界が矛盾するときだけ。
            return {};
        }
        active.push_back(line);
    }
    while (active.size() >= 3 && half_plane_outside(active[active.size() - 2], active.back(), active.front()))
    {
        active.pop_back();
    }
    if (active.size() >= 3)
    {
        half_plane_drop_front(active, active.back(), 3);
    }
    if (active.size() < 3)
    {
        return {};
    }

    std::vector<HalfPlaneRealVertex> result;
    result.reserve(active.size());
    for (std::size_t i = 0; i < active.size(); ++i)
    {
        const auto before = i == 0 ? active.size() - 1 : i - 1;
        const auto after = i + 1 == active.size() ? 0 : i + 1;
        if (half_plane_cross(active[i], active[after]) <= 0)
        {
            return {};
        }
        // 連続する交点が一致するとき、座標変換の前に重複を省く。
        if (!half_plane_side_at_vertex(active[before], active[i], active[after]).IsZero())
        {
            result.push_back(half_plane_vertex(active[i], active[after]));
        }
    }
    if (result.empty())
    {
        result.push_back(half_plane_vertex(active[0], active[1]));
    }
    return result;
}

inline void append_polygon_half_planes(std::vector<HalfPlane> &lines, std::span<const HalfPlaneVertex> vertices)
{
    for (const auto &p : vertices)
    {
        check_half_plane_vertex(p);
    }
    auto a = vertices.back();
    for (const auto &b : vertices)
    {
        const auto dx = b.first - a.first;
        const auto dy = b.second - a.second;
        lines.push_back({dy, -dx, a.first * b.second - a.second * b.first});
        a = b;
    }
}
}

[[nodiscard]] inline std::vector<std::pair<double, double>> half_plane_intersection(
    std::span<const HalfPlane> constraints,
    std::pair<std::int64_t, std::int64_t> lower, std::pair<std::int64_t, std::int64_t> upper)
{
    detail::check_half_plane_vertex(lower);
    detail::check_half_plane_vertex(upper);
    if (lower.first > upper.first || lower.second > upper.second)
    {
        throw std::invalid_argument("half_plane_intersection: rectangle order");
    }
    std::vector<HalfPlane> lines{{1, 0, upper.first}, {0, 1, upper.second}, {-1, 0, -lower.first}, {0, -1, -lower.second}};
    lines.reserve(constraints.size() + lines.size());
    for (const auto &line : constraints)
    {
        if (!detail::half_plane_within(line.a, detail::kHalfPlaneNormalLimit) ||
            !detail::half_plane_within(line.b, detail::kHalfPlaneNormalLimit) ||
            !detail::half_plane_within(line.c, detail::kHalfPlaneOffsetLimit))
        {
            throw std::out_of_range("half_plane_intersection: coefficient magnitude");
        }
        if (line.a != 0 || line.b != 0)
        {
            lines.push_back(line);
        }
        else if (line.c < 0)
        {
            return {};
        }
    }
    return detail::intersect_bounded_half_planes(std::move(lines));
}

[[nodiscard]] inline std::vector<std::pair<double, double>> convex_polygon_intersection(
    std::span<const std::pair<std::int64_t, std::int64_t>> a, std::span<const std::pair<std::int64_t, std::int64_t>> b)
{
    if (a.size() < 3 || b.size() < 3)
    {
        return {};
    }
    std::vector<HalfPlane> lines;
    lines.reserve(a.size() + b.size());
    detail::append_polygon_half_planes(lines, a);
    detail::append_polygon_half_planes(lines, b);
    constexpr auto limit = detail::kHalfPlanePointLimit;
    return half_plane_intersection(lines, {-limit, -limit}, {limit, limit});
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
