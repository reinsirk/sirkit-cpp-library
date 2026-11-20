// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <utility>
#include <vector>

int main() {
    const ::sirkit::geo::PointI a{0, 0}, b{4, 0}, c{0, 3};
    assert(a.dist2(b) == 16 && a.orient(b, c) == 12);
    assert((b.rotated90() == ::sirkit::geo::PointI{0, 4}));
    assert(::sirkit::geo::orientation(a, b, c) == 1);
    const ::sirkit::geo::PointLD p{3, 4};
    assert(std::abs(p.normalized().norm() - 1) < 1e-12L);
#ifdef SIRKIT_GEO_LINEAR_DEFINED
    const ::sirkit::geo::Segment<std::int64_t> first{{0, 0}, {4, 0}}, overlap{{2, 0}, {6, 0}}, singleton{{1, 0}, {1, 0}};
    assert(::sirkit::geo::intersection(first, overlap).kind == ::sirkit::geo::SegmentIntersectionKind::Overlap);
    assert(::sirkit::geo::intersection(first, singleton).kind == ::sirkit::geo::SegmentIntersectionKind::Point);
    assert(::sirkit::geo::intersection(::sirkit::geo::Line<std::int64_t>{{0, 0}, {1, 0}}, ::sirkit::geo::Line<std::int64_t>{{0, 1}, {1, 1}}).kind == ::sirkit::geo::LineIntersectionKind::None);
    assert(std::abs(::sirkit::geo::distance(::sirkit::geo::PointI{1, 2}, singleton) - 2) < 1e-12L);
#endif
#ifdef SIRKIT_GEO_POLYGON_DEFINED
    const auto hull = ::sirkit::geo::convex_hull(std::vector<::sirkit::geo::PointI>{a, b, c, {1, 1}, a});
    assert(hull.size() == 3 && ::sirkit::geo::area(hull) == 6);
    assert(::sirkit::geo::contains(hull, ::sirkit::geo::PointI{1, 1}) == ::sirkit::geo::Location::Inside);
    assert(::sirkit::geo::contains(hull, a) == ::sirkit::geo::Location::Boundary);
    auto center = ::sirkit::geo::centroid(hull);
    assert(center && std::abs(center->x - 4.0L / 3) < 1e-12L && std::abs(center->y - 1) < 1e-12L);
    assert(!::sirkit::geo::centroid(std::vector<::sirkit::geo::PointI>{a, b}));
    assert(::sirkit::geo::convex_hull(std::vector<::sirkit::geo::PointI>{{0, 0}, {1, 0}, {2, 0}}).size() == 2);
#endif
#ifdef SIRKIT_GEO_CIRCLE_DEFINED
    const auto circle = ::sirkit::geo::circumcircle(a, b, c);
    assert(circle && std::abs(circle->radius - 2.5L) < 1e-12L);
    assert(!::sirkit::geo::circumcircle(a, a, b));
    const ::sirkit::geo::Circle unit{{0, 0}, 1};
    assert(::sirkit::geo::intersection(unit, ::sirkit::geo::Circle{{3, 0}, 1}).kind == ::sirkit::geo::CircleIntersectionKind::None);
    assert(::sirkit::geo::intersection(unit, unit).kind == ::sirkit::geo::CircleIntersectionKind::Coincident);
    assert(::sirkit::geo::intersection(unit, ::sirkit::geo::Circle{{2, 0}, 1}).kind == ::sirkit::geo::CircleIntersectionKind::Tangent);
    assert(::sirkit::geo::intersection(unit, ::sirkit::geo::Segment<long double>{{-2, 0}, {2, 0}}).size() == 2);
    assert(::sirkit::geo::intersection(unit, ::sirkit::geo::Segment<long double>{{2, 0}, {3, 0}}).empty());
#endif
    std::cout << "OK\n";
}
