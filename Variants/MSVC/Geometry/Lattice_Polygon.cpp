// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Geometry/Lattice_Polygon.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/lattice_polygon.md
//
// API一覧
// lattice_polygon_points(vertices): 整数頂点の単純多角形について、内部と境界の格子点を数える。
// 頂点はpair<int64_t,int64_t>の境界順。時計回り・反時計回り、閉路先頭の再掲も可。
// 各軸の最大座標差は2^32-1以下。超過時はoverflow_error。空、点、一直線上の入力にも対応。
// LatticePolygonPoints{interior,boundary}。点や線分のinteriorは0。total()は合計、uint64_t超過でoverflow_error。
// 頂点数n、最大座標差wとしてO(n log(w+2))時間、O(1)追加領域。整数のみで正確に計算する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_LATTICE_POLYGON_DEFINED
#define SIRKIT_LATTICE_POLYGON_DEFINED

namespace detail
{
using LatticePolygonVertex = std::pair<std::int64_t, std::int64_t>;

inline std::uint64_t lattice_polygon_checked_add(std::uint64_t a, std::uint64_t b)
{
    if (b > std::numeric_limits<std::uint64_t>::max() - a)
    {
        throw std::overflow_error("lattice_polygon_points: count out of range");
    }
    return a + b;
}

inline std::int64_t lattice_polygon_center(std::int64_t first, std::int64_t last)
{
    const auto width = static_cast<std::uint64_t>(last) - static_cast<std::uint64_t>(first);
    if (width > std::numeric_limits<std::uint32_t>::max())
    {
        throw std::overflow_error("lattice_polygon_points: coordinate span out of range");
    }
    // 中心を切り上げ、座標差をint32_tの範囲におさめる。
    return first + static_cast<std::int64_t>((width + 1) / 2);
}

inline std::uint64_t lattice_polygon_difference(std::int64_t a, std::int64_t b) noexcept
{
    if (a < b)
    {
        std::swap(a, b);
    }
    return static_cast<std::uint64_t>(a) - static_cast<std::uint64_t>(b);
}

struct LatticePolygonDoubleArea
{
    // 128ビット2の補数を2語に分ける。各交差積はint64_tに収まる。
    std::uint64_t high = 0;
    std::uint64_t low = 0;

    void append(std::int64_t area) noexcept
    {
        const auto before = low;
        low += static_cast<std::uint64_t>(area);
        high += static_cast<std::uint64_t>(area < 0 ? -1 : 0) + (low < before);
    }

    void make_absolute() noexcept
    {
        if ((high >> 63) != 0)
        {
            high = ~high + (low == 0);
            low = std::uint64_t{0} - low;
        }
    }

    [[nodiscard]] std::uint64_t count_interior(std::uint64_t boundary) const
    {
        auto adjusted = *this;
        adjusted.append(2);
        if (adjusted.high == 0 && adjusted.low < boundary)
        {
            throw std::invalid_argument("lattice_polygon_points: inconsistent boundary");
        }
        adjusted.high -= adjusted.low < boundary;
        adjusted.low -= boundary;
        if (adjusted.high > 1)
        {
            throw std::overflow_error("lattice_polygon_points: count out of range");
        }
        return (adjusted.high << 63) | (adjusted.low >> 1);
    }
};
}

struct LatticePolygonPoints
{
    std::uint64_t interior;
    std::uint64_t boundary;

    [[nodiscard]] std::uint64_t total() const
    {
        return detail::lattice_polygon_checked_add(interior, boundary);
    }
};

[[nodiscard]] inline LatticePolygonPoints lattice_polygon_points(
    std::span<const std::pair<std::int64_t, std::int64_t>> vertices)
{
    using Vertex = detail::LatticePolygonVertex;
    if (vertices.empty())
    {
        return {0, 0};
    }
    const auto [left, right] = std::ranges::minmax_element(vertices, {}, &Vertex::first);
    const auto [bottom, top] = std::ranges::minmax_element(vertices, {}, &Vertex::second);
    const auto center_x = detail::lattice_polygon_center(left->first, right->first);
    const auto center_y = detail::lattice_polygon_center(bottom->second, top->second);
    const auto centered = [&](const Vertex &p)
    {
        return Vertex{p.first - center_x, p.second - center_y};
    };
    const auto segment_gcd = [](const Vertex &a, const Vertex &b)
    {
        return std::gcd(detail::lattice_polygon_difference(a.first, b.first),
                        detail::lattice_polygon_difference(a.second, b.second));
    };
    detail::LatticePolygonDoubleArea area;
    std::uint64_t boundary = 0;
    auto previous = centered(vertices.back());
    for (const auto &point : vertices)
    {
        const auto current = centered(point);
        area.append(previous.first * current.second - previous.second * current.first);
        boundary = detail::lattice_polygon_checked_add(boundary, segment_gcd(previous, current));
        previous = current;
    }
    if (area.high == 0 && area.low == 0)
    {
        const auto [first, last] = std::ranges::minmax_element(vertices);
        return {0, segment_gcd(*first, *last) + 1};
    }
    area.make_absolute();
    return {area.count_interior(boundary), boundary};
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
