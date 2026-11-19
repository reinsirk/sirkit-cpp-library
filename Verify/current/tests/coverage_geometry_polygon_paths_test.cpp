// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TEST_POLYGON_SOURCE
#define SIRKIT_TEST_POLYGON_SOURCE "../../../Geometry/Polygon.cpp"
#endif
#include SIRKIT_TEST_POLYGON_SOURCE

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <vector>

namespace {
using namespace sirkit::geo;

template<class Error, class Operation>
void rejects(Operation operation) {
    try {
        operation();
    } catch (const Error&) {
        return;
    }
    assert(false);
}

void point_operations() {
    const PointI p{3, 4}, q{0, 0};
    const std::vector<PointI> rotations{{3, 4}, {-4, 3}, {-3, -4}, {4, -3}};
    for (int k = -4; k <= 4; ++k) assert(p.rotated90(k) == rotations[(k + 4) % 4]);
    const PointI sum{4, 2}, doubled{6, 8};
    assert((p + PointI{1, -2} == sum));
    assert(p * 2 == doubled && p.dist2(q) == 25 && p.dist(q) == 5);
    assert((PointLD{6, 8} / 2.0L == PointLD{3, 4}));
    const std::vector<PointI> points{p, q, sum, doubled};
    assert(points[0] + points[1] == p);
    assert(points[0] * 2 == points[3]);
    assert(points[0].dist2(points[1]) == 25);
    std::ostringstream text;
    text << p;
    assert(text.str() == "(3,4)");
    rejects<std::domain_error>([] { (void)(PointLD{1, 2} / 0.0L); });
    rejects<std::domain_error>([] { (void)PointLD{}.normalized(); });
#ifndef SKIP_RUNTIME_CHECKS
    const auto infinity = std::numeric_limits<long double>::infinity();
    rejects<std::domain_error>([&] { (void)PointLD{infinity, 1}.normalized(); });
    rejects<std::domain_error>([&] { (void)PointLD{1, infinity}.normalized(); });
#endif
}

using Coordinate = PointI::Value;
void check_rectangle(Coordinate width, Coordinate height) {
    const std::vector<PointI> expected{{0, 0}, {width, 0}, {width, height}, {0, height}};
    auto data = expected;
    for (int i = 0; i < 2100; ++i) data.push_back({width * (i % 5) / 4, height * (i % 7) / 6});
    assert(sirkit::geo::convex_hull(data) == expected);
    std::sort(data.begin(), data.end());
    assert(sirkit::convex_hull_sorted(data) == expected);
}

void polygon_operations() {
    const auto hull_capacity = std::vector<PointI>{}.max_size();
    assert(sirkit::detail::convex_hull_reserve_size(hull_capacity - 1, hull_capacity) == hull_capacity);
    rejects<std::length_error>([&] {
        (void)sirkit::detail::convex_hull_reserve_size(hull_capacity, hull_capacity);
    });
    constexpr auto count_limit = std::numeric_limits<std::uint32_t>::max();
    if constexpr (std::numeric_limits<std::size_t>::digits >= std::numeric_limits<std::uint32_t>::digits)
        assert(sirkit::detail::convex_hull_combined_radix_counts(count_limit, 6));
    if constexpr (std::numeric_limits<std::size_t>::digits > std::numeric_limits<std::uint32_t>::digits) {
        // 32ビット度数表の上限を越えると、桁ごとのsize_t集計を選択する。
        const auto too_many = static_cast<std::size_t>(std::uint64_t{count_limit} + 1);
        assert(!sirkit::detail::convex_hull_combined_radix_counts(too_many, 6));
    }

    const std::vector<PointI> collinear{{0, 0}, {1, 1}, {2, 2}};
    assert(!centroid(collinear));
    assert(contains(std::vector<PointI>{{1, 1}}, PointI{1, 1}) == Location::Boundary);
    assert(contains(std::vector<PointI>{{1, 1}}, PointI{0, 1}) == Location::Outside);
    const std::vector<PointD> square{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
    assert(contains(square, PointD{1, 0}) == Location::Boundary);
    assert(contains(square, PointD{0.5, 0.5}) == Location::Inside);
    assert(contains(square, PointD{3, 0}) == Location::Outside);
    const Line<Coordinate> cutter{{0, 2}, {2, 0}};
    assert(convex_cut(std::vector<PointI>{}, cutter).empty());
    rejects<std::domain_error>([&] { (void)convex_cut(collinear, Line<Coordinate>{{0, 0}, {0, 0}}); });
    const std::vector<PointI> closed{{0, 0}, {2, 0}, {2, 0}, {2, 2}, {0, 2}, {0, 0}};
    const std::vector<RealPoint> expected{{2, 0}, {2, 2}, {0, 2}};
    assert(convex_cut(closed, cutter) == expected);
    const std::vector<RealPoint> expected_closed{{0, 0}, {2, 0}, {2, 2}, {0, 2}};
    assert((convex_cut(closed, Line<Coordinate>{{0, -1}, {2, -1}}) == expected_closed));
    assert((convex_cut(closed, Line<Coordinate>{{0, 3}, {2, 3}}).empty()));
    assert((!on_segment(PointI{1, 1}, Segment<Coordinate>{{0, 0}, {2, 0}})));

#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::invalid_argument>([] { (void)sirkit::convex_hull_sorted(std::vector<PointI>{{2, 0}, {0, 0}}); });
#endif
    check_rectangle(100, 10);
#if defined(__SIZEOF_INT128__)
    constexpr std::int64_t wide = std::int64_t{1} << 35;
    check_rectangle(wide, 10);
    check_rectangle(10, wide);
    check_rectangle(wide, wide);
    constexpr std::int64_t half_span = std::int64_t{1} << 62;
    const std::vector<PointI> tall{{0, -half_span}, {1, -half_span}, {1, half_span}, {0, half_span}};
    assert(sirkit::geo::convex_hull(tall) == tall);
#endif
}

} // namespace

int main() {
    point_operations();
    polygon_operations();
}
