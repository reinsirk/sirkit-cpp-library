// SPDX-License-Identifier: CC0-1.0
#include "../../../DataStructure/line-container-2d.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>

namespace {

using Table = sirkit::LineContainer2D;
struct CountMember {
    friend constexpr auto member_pointer(CountMember);
};
template<class Tag, auto Member>
struct MemberAccess {
    friend constexpr auto member_pointer(Tag) { return Member; }
};
template struct MemberAccess<CountMember, &Table::count_>;

template <class Error, class Call>
void rejects(Call call) {
    try {
        call();
    } catch (const Error&) {
        return;
    }
    assert(false);
}

void capacity_checks() {
    Table points;
    points.add(7, -3);
    constexpr auto maximum = std::numeric_limits<std::size_t>::max();
    // 同じ座標を繰り返し追加した状態。点数だけを上限直前まで進める。
    points.*member_pointer(CountMember{}) = maximum - 1;
    points.add(7, -3);
    assert(points.size() == maximum);
    rejects<std::length_error>([&] { points.add(100, 150); });
    assert(points.size() == maximum && points.max_ld(1, 1) == 4 && points.min_ld(1, 1) == 4);
}

void integer_boundary() {
    using Integer = std::int64_t;
    const auto minimum = std::numeric_limits<Integer>::min();
    const auto maximum = std::numeric_limits<Integer>::max();
    sirkit::MaxLineContainer<Integer> lines;
    lines.add(0, minimum);
    lines.add(1, maximum);
    // 交点はTの下限より小さく、最小の照会座標でも後者だけが選ばれる。
    assert(lines.size() == 1 && lines.query(minimum) == -1);
    assert(lines.query_line(minimum) == (std::pair<Integer, Integer>{1, maximum}));
}

void close_intersections() {
    using Real = long double;
    using Point = std::pair<Real, Real>;
    if constexpr (std::numeric_limits<Real>::digits != 64) return;
    const std::array<Point, 4> coordinates{{
        {-0x8.52722f319b14a49p+83L, 0x9.10b8d717409ddbcp+71L},
        {-0xb.bb3dcd755e25ecbp+84L, 0xc.c7781bc73b25608p+72L},
        {-0xc.33736cdf266ba49p-73L, 0x0p+0L},
        {-0xd.df9ca6a25bffdbfp+82L, 0xf.1cd0ef79d389322p+70L},
    }};
    // 旧境界と再計算した境界の丸めが異なるため、追加時に一時候補の除去を通る。
    Table hull;
    for (const auto& [x, y] : coordinates) hull.add(x, y);
    const auto infinity = std::numeric_limits<Real>::infinity();
    const auto center = -coordinates.front().first / coordinates.front().second;
    for (const Real slope : {0.0L, std::nextafter(center, -infinity), center,
                             std::nextafter(center, infinity), center * 2}) {
        Real minimum = infinity, maximum = -infinity, scale = 1;
        for (const auto& [x, y] : coordinates) {
            const auto value = std::fma(slope, y, x);
            minimum = std::min(minimum, value);
            maximum = std::max(maximum, value);
            scale = std::max(scale, std::abs(x) + std::abs(slope * y));
        }
        const auto lo = hull.min_with_point(1, slope);
        const auto hi = hull.max_with_point(1, slope);
        const auto tolerance = 16 * std::numeric_limits<Real>::epsilon() * scale;
        assert(std::ranges::find(coordinates, Point{lo.x, lo.y}) != coordinates.end());
        assert(std::ranges::find(coordinates, Point{hi.x, hi.y}) != coordinates.end());
        assert(std::abs(std::fma(slope, lo.y, lo.x) - minimum) <= tolerance);
        assert(std::abs(std::fma(slope, hi.y, hi.x) - maximum) <= tolerance);
        assert(std::abs(lo.value - minimum) <= tolerance && std::abs(hi.value - maximum) <= tolerance);
    }
}

void unit_boundary_paths() {
    using Real = long double;
    const auto huge = std::ldexp(1.0L, std::numeric_limits<Real>::max_exponent / 2 + 1);
    for (const Real sign : {-1.0L, 1.0L}) {
        Table hull;
        hull.add(huge, -sign * huge);
        hull.add(-huge, sign * huge);
        for (const Real a : {-2.0L, -0.5L, 0.0L, 0.5L, 2.0L}) {
            for (const Real b : {-2.0L, -0.5L, 0.0L, 0.5L, 2.0L}) {
                const Real expected = std::abs((a - sign * b) * huge);
                assert(hull.max_ld(a, b) == expected && hull.min_ld(a, b) == -expected);
            }
        }
        // 積ごとには表せないが、加算すると厳密に相殺する。
        assert(hull.min_ld(huge, sign * huge) == 0);
        assert(hull.max_ld(-huge, -sign * huge) == 0);
        rejects<std::overflow_error>([&] {
            (void)hull.min_ld(std::numeric_limits<Real>::min(), std::numeric_limits<Real>::max());
        });
    }
    Table single;
    single.add(huge, huge);
    const auto infinity = std::numeric_limits<Real>::infinity();
    assert(single.max_ld(huge, huge) == infinity);
    assert(single.min_ld(-huge, -huge) == -infinity);
}

} // namespace

int main() {
    using Point = std::pair<long double, long double>;
    sirkit::LineContainer2D points;
    sirkit::MinLineContainer<long double> minimum;
    sirkit::MaxLineContainer<long double> maximum;
    std::vector<Point> reference;
    const std::vector<Point> insertions{
        {9, 0}, {-2, 0}, {11, 0}, {11, 0}, {0, -7}, {0, 9},
        {6, -3}, {2, 4}, {-5, -3}, {12, -3}, {8, 4}, {-12, 4}
    };
#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::out_of_range>([&] { (void)minimum.query(0); });
    const auto infinite = std::numeric_limits<long double>::infinity();
    rejects<std::invalid_argument>([&] { minimum.add(infinite, 0); });
    rejects<std::invalid_argument>([&] { maximum.add(0, infinite); });
    assert(minimum.empty() && maximum.empty());
#endif
    for (const auto& [x, y] : insertions) {
        points.add(x, y);
        minimum.add(y, x);
        maximum.add(y, x);
        reference.emplace_back(x, y);
        for (const int slope : {-16, -4, -1, 0, 1, 4, 16}) {
            long double lo = std::numeric_limits<long double>::infinity(), hi = -lo;
            for (const auto& [px, py] : reference) {
                const auto value = px + slope * py;
                lo = std::min(lo, value);
                hi = std::max(hi, value);
            }
            assert(minimum.query(slope) == lo && maximum.query(slope) == hi);
            assert(points.min_ld(1, slope) == lo && points.max_ld(1, slope) == hi);
            const auto min_point = points.min_with_point(1.0L, slope);
            const auto max_point = points.max_with_point(1.0L, slope);
            assert(min_point.value == lo && max_point.value == hi);
            assert(min_point.x + slope * min_point.y == lo);
            assert(max_point.x + slope * max_point.y == hi);
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    rejects<std::invalid_argument>([&] { (void)maximum.query(infinite); });
#endif
    // 有限係数でも、係数比の範囲外は検査設定にかかわらず例外になる。
    const auto small = std::numeric_limits<long double>::min();
    const auto large = std::numeric_limits<long double>::max();
    try {
        (void)points.argmax(small, large);
        assert(false);
    } catch (const std::overflow_error&) {
    }
    try {
        (void)points.min_ld(small, large);
        assert(false);
    } catch (const std::overflow_error&) {
    }
    capacity_checks();
    integer_boundary();
    close_intersections();
    unit_boundary_paths();
}
