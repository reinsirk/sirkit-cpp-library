// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/geometory.cpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <utility>
#include <vector>

using namespace sirkit;

int main() {
    using ::sirkit::geo::PointI;
    const PointI a{0, 0}, b{4, 0}, c{0, 3};
    assert(a.dist(b) == 4);
    assert(::sirkit::geo::orientation(a, b, c) == 1);
    const ::sirkit::geo::Segment<std::int64_t> s{a, b}, t{{2, -1}, {2, 1}};
    assert(::sirkit::geo::intersects(s, t));
    const auto hit = ::sirkit::geo::intersection(s, t);
    assert(hit.kind == ::sirkit::geo::SegmentIntersectionKind::Point);
    assert(std::abs(hit.points[0].x - 2) < 1e-12L);
    const std::vector<PointI> points{a, b, c, {1, 1}};
    const auto hull = ::sirkit::geo::convex_hull(points);
    assert(hull.size() == 3 && ::sirkit::geo::area(hull) == 6);
    auto sorted_points = points;
    std::sort(sorted_points.begin(), sorted_points.end());
    assert(convex_hull_sorted(std::move(sorted_points)) == hull);
    assert(::sirkit::geo::contains(hull, PointI{1, 1}) == ::sirkit::geo::Location::Inside);
    const auto cut = ::sirkit::geo::convex_cut(hull, ::sirkit::geo::Line<std::int64_t>{{2, -1}, {2, 4}});
    assert(std::abs(::sirkit::geo::area(cut) - 4.5L) < 1e-12L);
    const auto circle = ::sirkit::geo::circumcircle(a, b, c);
    assert(circle && std::abs(circle->radius - 2.5L) < 1e-12L);
    const auto circle_hit = ::sirkit::geo::intersection(*circle, ::sirkit::geo::Line<long double>{::sirkit::geo::to_real(a), ::sirkit::geo::to_real(b)});
    assert(circle_hit.size() == 2);
    const ::sirkit::geo::Circle left{{0, 0}, 5}, right{{6, 0}, 5};
    // 交点が不要なときは、確保せず交差種別だけを求める。
    assert(circle_intersection_kind(left, right) == ::sirkit::geo::CircleIntersectionKind::TwoPoints);
    // 三角形は配列を用意せずに面積を求められる。
    assert(signed_triangle_area2(a, b, c) == 12);
    assert(triangle_area(a, b, c) == 6);
}
