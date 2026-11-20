// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TEST_SEGMENT_ALTERNATE_SOURCE
#define SIRKIT_TEST_SEGMENT_ALTERNATE_SOURCE "../../../Geometry/geometory.cpp"
#endif
#include SIRKIT_TEST_SEGMENT_ALTERNATE_SOURCE

#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <stdexcept>

namespace {
template<class Point>
bool ref_intersects(Point a, Point b, Point c, Point d) {
    auto cross = [](Point p, Point q, Point r) {
        return (q.x - p.x) * (r.y - p.y) - (q.y - p.y) * (r.x - p.x);
    };
    auto overlap = [](int p, int q, int r, int s) {
        return std::max(std::min(p, q), std::min(r, s)) <= std::min(std::max(p, q), std::max(r, s));
    };
    return overlap(a.x, b.x, c.x, d.x) && overlap(a.y, b.y, c.y, d.y) &&
           cross(a, b, c) * cross(a, b, d) <= 0 && cross(c, d, a) * cross(c, d, b) <= 0;
}

template<class Calc>
void check_calculation_type() {
    using Point = sirkit::geo::Point<int, Calc>;
    using Segment = sirkit::geo::Segment<int, Calc>;
    std::array<Point, 9> grid;
    for (int i = 0; i < 9; ++i) grid[i] = {i / 3 - 1, i % 3 - 1};
    for (Point a : grid) for (Point b : grid) for (Point c : grid) for (Point d : grid) {
        assert(sirkit::geo::intersects(Segment{a, b}, Segment{c, d}) == ref_intersects(a, b, c, d));
    }
}
} // namespace

int main() {
    assert(sirkit::geo::predicate_sign<int>(-1, 100) == -1);
    assert(sirkit::geo::predicate_sign<int>(0, 100) == 0);
    assert(sirkit::geo::predicate_sign<int>(1, 100) == 1);
    check_calculation_type<long double>();
    check_calculation_type<boost::multiprecision::cpp_rational>();
    const sirkit::geo::PointI query{3, 4};
    const sirkit::geo::Segment<std::int64_t> singleton{{0, 0}, {0, 0}}, horizontal{{0, 0}, {6, 0}};
    assert(sirkit::geo::distance(query, singleton) == 5);
    assert(sirkit::geo::distance(query, horizontal) == 4);
    bool rejected = false;
    try {
        (void)projection(query, sirkit::geo::Line<std::int64_t>{{0, 0}, {0, 0}});
    } catch (const std::domain_error&) {
        rejected = true;
    }
    assert(rejected);
}
