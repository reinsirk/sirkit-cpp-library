// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TEST_CIRCLE_TOLERANCE_SOURCE
#define SIRKIT_TEST_CIRCLE_TOLERANCE_SOURCE "../../../Geometry/Circle.cpp"
#endif
#include SIRKIT_TEST_CIRCLE_TOLERANCE_SOURCE

#include <cassert>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {
using namespace sirkit::geo;

template<class Operation>
void rejects(Operation operation) {
    try {
        operation();
    } catch (const std::domain_error&) {
        return;
    }
    assert(false);
}
} // namespace

int main() {
    using namespace sirkit::geo;
    const PointI p{3, 4}, origin{0, 0}, raised{3, 5};
    assert(p.dist(origin) == 5);
    assert((origin.orient(PointI{1, 0}, PointI{0, 1}) == 1));
    const std::vector<PointI> rotation{{3, 4}, {-4, 3}, {-3, -4}, {4, -3}};
    for (int turns = -4; turns < 4; ++turns) assert(p.rotated90(turns) == rotation[(turns + 4) % 4]);
    assert(p != raised && p < raised);
    const std::vector<PointI> integer_segment{{0, 0}, {2, 0}, {1, 0}, {3, 0}, {1, 1}};
    const Segment<PointI::Value> segment{integer_segment[0], integer_segment[1]};
    assert(segment.a != segment.b && segment.a < segment.b);
    assert(integer_segment[2] != integer_segment[4]);
    assert(segment.a.orient(segment.b, integer_segment[4]) == 2);
    assert(on_segment(integer_segment[2], segment) && !on_segment(integer_segment[3], segment));
    for (const int value : {-2, 0, 2}) {
        assert((predicate_sign<int>(value, 100, Epsilon{100, 100}) == value / 2));
    }
    assert((!on_segment(PointI{1, 1}, Segment<PointI::Value>{{0, 0}, {2, 0}})));
    rejects([] { (void)PointLD{}.normalized(); });
    rejects([] { (void)(PointLD{1, 2} / 0.0L); });
#ifndef SKIP_RUNTIME_CHECKS
    const auto infinity = std::numeric_limits<long double>::infinity();
    rejects([&] { (void)PointLD{infinity, 0}.normalized(); });
    rejects([&] { (void)PointLD{0, infinity}.normalized(); });
#endif
    const Circle outer{{0, 0}, 5}, inner{{4.25L, 0}, 1};
    // 大きい円だけが接線まで許容幅以内でも、2つの交点は残る。
    const Epsilon tolerance{0.1L, 0};
    const auto result = intersection(outer, inner, tolerance);
    assert(result.kind == CircleIntersectionKind::TwoPoints);
    assert(sirkit::circle_intersection_kind(outer, inner, tolerance) == result.kind);
    assert(result.points.size() == 2 && result.points[0].y * result.points[1].y < 0);
    constexpr long double cross_x = 673.0L / 136;
    for (const auto point : result.points) {
        assert(std::abs(point.x - cross_x) < 1e-12L);
        assert(std::abs(point.dist2(outer.center) - 25) < 1e-12L);
        assert(std::abs(point.dist2(inner.center) - 1) < 1e-12L);
    }

    const Circle point{{0, 0}, 0};
    assert(intersection(point, point).kind == CircleIntersectionKind::Tangent);
    const auto concentric = intersection(Circle{{0, 0}, 0.5L}, point, Epsilon{0.5L, 0});
    const std::vector<RealPoint> center{{0, 0}};
    assert(concentric.kind == CircleIntersectionKind::Tangent && concentric.points == center);
    assert(intersection(outer, Line<long double>{{0, 6}, {2, 6}}).empty());
    rejects([&] { (void)intersection(outer, Line<long double>{{1, 1}, {1, 1}}); });
}
