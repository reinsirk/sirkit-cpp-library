// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Geometry/Circle_Polygon_Intersection.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/circle_polygon_intersection.md
//
// API一覧
// circle_polygon_intersection_area(vertices, radius, center={0,0}): 円板と単純多角形の共通部分の面積。
// 頂点はpair<double,double>の周回順。凸・凹、時計回り・反時計回りに対応。重複頂点可。
// 半径0または頂点3個未満は面積0。半径と中心相対の各座標は有限で絶対値1e140以下、半径は非負。
// 頂点数nに対しO(n)時間、O(1)追加領域。浮動小数点の丸めを含む。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cmath>
#include <numbers>
#include <span>
#include <stdexcept>
#include <utility>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CIRCLE_POLYGON_INTERSECTION_DEFINED
#define SIRKIT_CIRCLE_POLYGON_INTERSECTION_DEFINED

namespace detail
{
using CircleClipPoint = std::pair<double, double>;

inline double circle_clip_cross(const CircleClipPoint &a, const CircleClipPoint &b) noexcept
{
    return a.first * b.second - a.second * b.first;
}

inline double circle_clip_dot(const CircleClipPoint &a, const CircleClipPoint &b) noexcept
{
    return a.first * b.first + a.second * b.second;
}

inline double circle_clip_angle(CircleClipPoint a, CircleClipPoint b)
{
    // 円弧の端点は非零。角度の計算では長さの桁の差を取り除く。
    const auto scale_a = std::max(std::abs(a.first), std::abs(a.second));
    const auto scale_b = std::max(std::abs(b.first), std::abs(b.second));
    a = {a.first / scale_a, a.second / scale_a};
    b = {b.first / scale_b, b.second / scale_b};
    return std::atan2(circle_clip_cross(a, b), circle_clip_dot(a, b));
}

struct CircleClipContribution
{
    double twice_area;
    bool has_segment;
};

inline CircleClipContribution circle_clip_edge(const CircleClipPoint &a, const CircleClipPoint &b,
                                               double radius, double radius_squared)
{
    if (a == b)
    {
        return {0, false};
    }
    if (circle_clip_dot(a, a) <= radius_squared && circle_clip_dot(b, b) <= radius_squared)
    {
        return {circle_clip_cross(a, b), true};
    }

    const double dx = b.first - a.first;
    const double dy = b.second - a.second;
    const double length = std::hypot(dx, dy);
    const CircleClipPoint direction{dx / length, dy / length};
    const double h = circle_clip_cross(direction, a);
    const double distance = std::abs(h);
    if (distance >= radius)
    {
        return {radius_squared * circle_clip_angle(a, b), false};
    }
    const double half = std::sqrt((radius - distance) * (radius + distance));
    const double first = circle_clip_dot(a, direction);
    const double last = circle_clip_dot(b, direction);
    const double enter = std::max(first, -half);
    const double leave = std::min(last, half);
    if (enter >= leave)
    {
        return {radius_squared * circle_clip_angle(a, b), false};
    }

    // 交点は線分の端から補間せず、円の中心に近い点から計算する。
    const auto point_at = [&](double t) -> CircleClipPoint
    {
        return {t * direction.first - h * direction.second,
                t * direction.second + h * direction.first};
    };
    auto start = a;
    auto end = b;
    double arcs = 0;
    if (enter > first)
    {
        start = point_at(enter);
        arcs += circle_clip_angle(a, start);
    }
    if (leave < last)
    {
        end = point_at(leave);
        arcs += circle_clip_angle(end, b);
    }
    return {circle_clip_cross(start, end) + radius_squared * arcs, true};
}
}

[[nodiscard]] inline double circle_polygon_intersection_area(
    std::span<const std::pair<double, double>> vertices, double radius,
    std::pair<double, double> center = {})
{
    constexpr double kLimit = 1e140; // 積・累積和をdoubleの範囲に収める。
    if (!(0 <= radius && radius <= kLimit) ||
        !std::isfinite(center.first) || !std::isfinite(center.second))
    {
        throw std::invalid_argument("circle_polygon_intersection_area: circle");
    }
    const double radius_squared = radius * radius;
    if (vertices.size() < 3 || radius_squared == 0)
    {
        return 0;
    }
    const auto relative = [&](const auto &p) -> detail::CircleClipPoint
    {
        const double x = p.first - center.first;
        const double y = p.second - center.second;
        if (!(std::abs(x) <= kLimit) || !(std::abs(y) <= kLimit))
        {
            throw std::out_of_range("circle_polygon_intersection_area: relative coordinate");
        }
        return {x, y};
    };

    const double disk_area = std::numbers::pi * radius_squared;
    double twice_area = 0;
    double error = 0;
    bool has_segment = false;
    auto from = relative(vertices.back());
    for (const auto &vertex : vertices)
    {
        const auto to = relative(vertex);
        const auto edge = detail::circle_clip_edge(from, to, radius, radius_squared);
        const double summand = edge.twice_area - error;
        const double next = twice_area + summand;
        error = (next - twice_area) - summand;
        twice_area = next;
        has_segment |= edge.has_segment;
        from = to;
    }
    // 円板の内部に入る辺がなければ、境界が囲む角度は0または1周。
    if (!has_segment)
    {
        return std::abs(twice_area) > disk_area ? disk_area : 0;
    }
    return std::min(disk_area, std::abs(twice_area) * 0.5);
}
#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
