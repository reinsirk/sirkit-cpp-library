// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TEST_SEGMENT_SOURCE
#define SIRKIT_TEST_SEGMENT_SOURCE "../../../Geometry/Line_Segment.cpp"
#endif
#include SIRKIT_TEST_SEGMENT_SOURCE

#include <boost/multiprecision/cpp_int.hpp>
#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

using ScaledPoint = std::array<std::int64_t, 2>;
using Fraction = boost::multiprecision::cpp_rational;
using ExactPoint = std::array<Fraction, 2>;

ExactPoint difference(const ExactPoint& a, const ExactPoint& b) {
    return {a[0] - b[0], a[1] - b[1]};
}

Fraction cross(const ExactPoint& a, const ExactPoint& b) {
    return a[0] * b[1] - a[1] * b[0];
}

template<class T>
void check_near_parallel(const std::array<ScaledPoint, 4>& scaled, bool proper_crossing) {
    using namespace sirkit::geo;
    // 入力座標を8倍した整数から、交差と座標を有理数で独立に求める。
    std::array<ExactPoint, 4> exact;
    std::array<Point<T>, 4> input;
    for (std::size_t i = 0; i < scaled.size(); ++i) {
        const auto [x, y] = scaled[i];
        exact[i] = {Fraction{x} / 8, Fraction{y} / 8};
        input[i] = {static_cast<T>(x) / 8, static_cast<T>(y) / 8};
        assert(static_cast<std::int64_t>(input[i].x * 8) == x);
        assert(static_cast<std::int64_t>(input[i].y * 8) == y);
    }
    const auto u = difference(exact[1], exact[0]);
    const auto v = difference(exact[3], exact[2]);
    const auto w = difference(exact[2], exact[0]);
    const auto denominator = cross(u, v);
    assert(denominator != 0);
    const Fraction along_first = cross(w, v) / denominator;
    const Fraction along_second = cross(w, u) / denominator;
    assert((0 < along_first && along_first < 1 && 0 < along_second && along_second < 1) == proper_crossing);
    const ExactPoint exact_hit{exact[0][0] + along_first * u[0], exact[0][1] + along_first * u[1]};
    const RealPoint expected{exact_hit[0].convert_to<long double>(), exact_hit[1].convert_to<long double>()};
    const auto close = [](long double actual, long double reference) {
        return std::abs(actual - reference) <= 1e-10L * std::max(1.0L, std::abs(reference));
    };
    // 小さな正の許容幅でも、これらの入力では厳密な向きの符号は変わらない。
    for (const Epsilon eps : {Epsilon{0, 0}, Epsilon{1e-18L, 1e-30L}}) {
        for (int order = 0; order < 8; ++order) {
            Segment<T> first{input[0], input[1]}, second{input[2], input[3]};
            if (order & 1) std::swap(first.a, first.b);
            if (order & 2) std::swap(second.a, second.b);
            if (order & 4) std::swap(first, second);
            const auto result = intersection(first, second, eps);
            assert(intersects(first, second, eps) == proper_crossing);
            if (proper_crossing) {
                assert(result.kind == SegmentIntersectionKind::Point && result.points.size() == 1);
                assert(close(result.points[0].x, expected.x) && close(result.points[0].y, expected.y));
            } else {
                assert(result.kind == SegmentIntersectionKind::None && result.points.empty());
            }
        }
    }
}

void near_parallel_regression() {
    check_near_parallel<double>({{
        {-28062974346898368, 19435597857951336},
        {2438925390021304, -7244423134594080},
        {-28062974346898256, 19435597857951240},
        {10064400324251326, -13914428382730526}
    }}, true);
    check_near_parallel<double>({{
        {-114618281078368, 5260637003915008},
        {34583569380750088, 28277139481628288},
        {-13126438654263930, -3370551425227398},
        {51932663211664376, 39785390720484960}
    }}, false);
    // floatでは向きが零へ丸められる交差と、向きが反転する非交差を調べる。
    check_near_parallel<float>({{
        {57446960, 26119376}, {35331288, 54362800},
        {40860224, 47301920}, {27037908, 64954096}
    }}, true);
    check_near_parallel<float>({{
        {25706096, 1600048}, {69009648, -54804760},
        {31119036, -5450507}, {85248472, -75956568}
    }}, false);
    if constexpr (std::numeric_limits<long double>::digits >= 64) {
        check_near_parallel<long double>({{
            {-1162160133425929904, -1302288757936325768},
            {-2491072908737967216, 356417309484061376},
            {-829931939597920578, -1716965274791422558},
            {-3155529296393985869, 1185770343194254947}
        }}, true);
    }
}

void absolute_tolerance_boundary() {
    using namespace sirkit::geo;
    const double span = 0x1p26;
    const double half = span / 2;
    const double step = std::nextafter(half, span) - half;
    const Segment<double> first{{0, 0}, {span, span}};
    const Segment<double> second{{half, half + 3 * step}, {half, half + 1}};
    // 真の行列式は span × 3 step。絶対許容幅と等しければ端点接触になる。
    const long double unit = static_cast<long double>(span) * step;
    for (const long double width : {0.5L, 2.5L, 3.0L, 3.5L, 4.0L}) {
        const Epsilon eps{width * unit, 0};
        const bool touches = width >= 3;
        assert(intersects(first, second, eps) == touches);
        const auto hit = intersection(first, second, eps);
        if (touches) {
            assert(hit.kind == SegmentIntersectionKind::Point &&
                   hit.points == std::vector{to_real(second.a)});
        } else {
            assert(hit.kind == SegmentIntersectionKind::None && hit.points.empty());
        }
    }
}

void subnormal_endpoint_bounds() {
    using namespace sirkit::geo;
    const double unit = std::numeric_limits<double>::denorm_min();
    if (unit == 0) return;
    const Segment<double> base{{0, 0}, {2 * unit, 0}};
    const Segment<double> outside{{3 * unit, 0}, {4 * unit, 0}};
    const Segment<double> touching{{2 * unit, 0}, {3 * unit, 0}};
    const Epsilon strict{0, 0};
    // 両者は共線。座標の積が丸めでゼロになっても、区間外の点は含めない。
    assert(!intersects(base, outside, strict));
    assert(intersection(base, outside, strict).kind == SegmentIntersectionKind::None);
    assert(intersects(base, touching, strict));
    const auto hit = intersection(base, touching, strict);
    assert(hit.kind == SegmentIntersectionKind::Point && hit.points == std::vector{to_real(base.b)});
}

void floating_magnitude_boundaries() {
    using namespace sirkit::geo;
    const Epsilon strict{0, 0};
    const Segment<long double> base{{0, 0}, {7, 0}};
    const Segment<long double> crossing{{3.25L, -1}, {3.25L, 1}};
    const std::vector<RealPoint> expected{{3.25L, 0}};
    assert(intersects(base, crossing, strict));
    assert(intersection(base, crossing, strict).points == expected);
    assert(intersection(Segment<long double>{base.b, base.a}, crossing, strict).points == expected);
    assert(intersection(base, Segment<long double>{crossing.a, expected.front()}, {0, 1e-12L}).points == expected);

    // 許容幅の入力は有限でも、相対許容幅と行列式の尺度の積は無限大になり得る。
    const Epsilon huge{0, std::numeric_limits<long double>::max()};
    assert(intersects(base, crossing, huge));
    assert(intersection(base, crossing, huge).kind == SegmentIntersectionKind::Overlap);

    // 両方の積は有限だが、その絶対値の和を表せないときも向きを判定する。
    const long double big = std::ldexp(1.0L, std::numeric_limits<long double>::max_exponent - 1);
    const PointLD origin{0, 0}, diagonal{big, big}, above{1, 1.25L};
    assert(sirkit::detail::floating_segment_side(origin, diagonal, above, strict) == 1);
    assert(sirkit::detail::floating_segment_side(origin, diagonal, above, {0, 1}) == 0);

    const long double unit = std::numeric_limits<long double>::denorm_min();
    if (unit == 0 || 32 * unit >= std::numeric_limits<long double>::min()) return;
    const Segment<long double> axis{{0, 0}, {1, 0}};
    const Segment<long double> tiny{{0.25L, -8 * unit}, {0.25L, 8 * unit}};
    assert(intersects(axis, tiny, strict));
    const auto hit = intersection(axis, tiny, strict);
    assert((hit.kind == SegmentIntersectionKind::Point && hit.points == std::vector<RealPoint>{{0.25L, 0}}));
}

} // namespace

int main() {
    using namespace sirkit::geo;
    const PointI p{3, 4};
    const std::vector<PointI> expected{{3, 4}, {-4, 3}, {-3, -4}, {4, -3}};
    for (int turn = -9; turn <= 9; ++turn) {
        assert(p.rotated90(turn) == expected[(turn + 12) % 4]);
    }
    assert(p.dist2(PointI{0, 0}) == 25);
    assert((PointI{0, 0}.orient(PointI{6, 0}, PointI{3, 4}) == 24));

    const Line<PointI::Value> horizontal{{0, 3}, {4, 3}};
    const Line<PointI::Value> diagonal{{0, 0}, {5, 5}};
    const RealPoint cross{3, 3};
    const auto crossing = intersection(horizontal, diagonal);
    assert(crossing.kind == LineIntersectionKind::Point && crossing.point == cross);
    assert(intersection(diagonal, horizontal).point == cross);
    assert(intersection(horizontal, Line<PointI::Value>{{4, 3}, {6, 3}}).kind ==
           LineIntersectionKind::Coincident);
    assert(intersection(horizontal, Line<PointI::Value>{{4, 4}, {6, 4}}).kind ==
           LineIntersectionKind::None);
    bool degenerate = false;
    try {
        (void)intersection(horizontal, Line<PointI::Value>{{1, 2}, {1, 2}});
    } catch (const std::domain_error&) {
        degenerate = true;
    }
    assert(degenerate);

    const Segment<PointI::Value> base{{0, 3}, {4, 3}};
    const Segment<PointI::Value> through{{0, 0}, {5, 5}};
    const auto proper = intersection(base, through);
    assert(proper.kind == SegmentIntersectionKind::Point && proper.points == std::vector{cross});
    const auto overlap = intersection(base, Segment<PointI::Value>{{7, 3}, {2, 3}});
    const std::vector<RealPoint> ends{{2, 3}, {4, 3}};
    assert(overlap.kind == SegmentIntersectionKind::Overlap && overlap.points == ends);
    assert(intersection(base, Segment<PointI::Value>{{3, 3}, {3, 3}}).points == std::vector{cross});
    assert(intersection(base, Segment<PointI::Value>{{0, 2}, {4, 2}}).kind ==
           SegmentIntersectionKind::None);

    const std::vector<PointI> comparison{{3, 4}, {2, 5}, {3, 9}, {0, 0}, {6, 0}};
    assert(!(comparison[0] < comparison[1]));
    assert(comparison[0] < comparison[2]);
    assert(comparison[0].dist2(comparison[3]) == 25);
    assert(comparison[3].orient(comparison[4], comparison[0]) == 24);
    const Line<PointI::Value> vertical{{1, 0}, {1, 7}};
    const Segment<PointI::Value> vertical_segment{{1, 0}, {1, 7}};
    const RealPoint foot{1, 4};
    assert(projection(p, vertical) == foot && closest_point(p, vertical_segment) == foot);
    assert(distance(p, vertical) == 2);
    const Segment<PointI::Value> horizontal_segment{{0, 3}, {4, 3}};
    const PointI before{-1, 5}, after{5, 5}, zero{2, 1};
    const RealPoint first_end{0, 3}, second_end{4, 3}, zero_end{2, 1};
    assert(closest_point(before, horizontal_segment) == first_end);
    assert(closest_point(after, horizontal_segment) == second_end);
    assert(closest_point(p, Segment<PointI::Value>{zero, zero}) == zero_end);
    assert(distance(p, horizontal) == 1);
    assert(distance(p, vertical_segment) == 2);
    bool rejected_distance = false;
    try {
        (void)distance(p, Line<PointI::Value>{zero, zero});
    } catch (const std::domain_error&) {
        rejected_distance = true;
    }
    assert(rejected_distance);
    const auto reject_line = [&] {
        try { (void)intersection(Line<PointI::Value>{{1, 2}, {1, 2}}, horizontal); }
        catch (const std::domain_error&) { return true; }
        return false;
    };
    assert(reject_line());

    const Segment<long double> floating_base{{0, 0}, {4, 0}};
    const Segment<long double> end_touches{{1, 2}, {1, 0}};
    const Segment<long double> start_touches{{0, -1}, {0, 1}};
    assert(intersects(floating_base, end_touches));
    assert(intersects(floating_base, start_touches));
    const auto contact = intersection(floating_base, start_touches);
    const std::vector<RealPoint> at_start{{0, 0}};
    assert(contact.kind == SegmentIntersectionKind::Point && contact.points == at_start);
    assert(intersection(floating_base, Segment<long double>{{5, -2}, {5, 2}}).kind ==
           SegmentIntersectionKind::None);
    const Segment<long double> near_touch{{1, 1e-10L}, {1, 1}};
    const Epsilon strict{0, 0}, tolerant{1e-8L, 0};
    assert(!intersects(floating_base, near_touch, strict));
    assert(intersection(floating_base, near_touch, strict).kind == SegmentIntersectionKind::None);
    assert(intersects(floating_base, near_touch, tolerant));
    const auto tolerant_contact = intersection(floating_base, near_touch, tolerant);
    assert(tolerant_contact.kind == SegmentIntersectionKind::Point &&
           tolerant_contact.points == std::vector{near_touch.a});
    const auto reject_normalization = [](RealPoint point) {
        try { (void)point.normalized(); }
        catch (const std::domain_error&) { return true; }
        return false;
    };
    assert(reject_normalization({0, 0}));
#ifndef SKIP_RUNTIME_CHECKS
    const auto inf = std::numeric_limits<long double>::infinity();
    assert(reject_normalization({inf, 1}));
    assert(reject_normalization({1, inf}));
#endif
    near_parallel_regression();
    absolute_tolerance_boundary();
    subnormal_endpoint_bounds();
    floating_magnitude_boundaries();
}
