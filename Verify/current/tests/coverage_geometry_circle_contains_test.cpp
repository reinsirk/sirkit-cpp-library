// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TEST_CIRCLE_SOURCE
#define SIRKIT_TEST_CIRCLE_SOURCE "../../../Geometry/Circle.cpp"
#endif
#include SIRKIT_TEST_CIRCLE_SOURCE

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

int main() {
    using namespace sirkit::geo;
    const Epsilon exact{0, 0};
    const RealPoint center{2, 1}, boundary{5, 5};
    const Circle circle{center, 5};
    assert(contains(circle, center, exact) == Location::Inside);
    assert(contains(circle, boundary, exact) == Location::Boundary);
    assert(contains(circle, RealPoint{6, 5}, exact) == Location::Outside);
    assert(intersection(circle, Segment<long double>{boundary, boundary}, exact) == std::vector{boundary});
    assert(intersection(circle, Segment<long double>{center, center}, exact).empty());
    assert(contains(Circle{center, 0}, center, exact) == Location::Boundary);

    const Circle unit{{0, 0}, 1};
    const long double d = std::nextafter(2.0L, 0.0L);
    const auto near_tangent = intersection(unit, Circle{{d, 0}, 1}, Epsilon{1e-10L, 0});
    assert(near_tangent.kind == CircleIntersectionKind::Tangent);
    assert(near_tangent.points.size() == 1);
    assert(std::abs(near_tangent.points[0].x - 1) < 1e-10L && near_tangent.points[0].y == 0);
    const Circle point{{1, 0}, 0};
    const RealPoint touch{1, 0};
    assert(intersection(point, unit, exact).points == std::vector{touch});
    assert(intersection(unit, point, exact).points == std::vector{touch});
    assert(intersection(point, point, exact).points == std::vector{touch});
#ifndef SKIP_RUNTIME_CHECKS
    const long double infinity = std::numeric_limits<long double>::infinity();
    for (RealPoint invalid : {RealPoint{infinity, 0}, RealPoint{0, infinity}}) {
        bool caught = false;
        try {
            (void)contains(circle, invalid, exact);
        } catch (const std::domain_error&) {
            caught = true;
        }
        assert(caught);
    }
#endif
}
