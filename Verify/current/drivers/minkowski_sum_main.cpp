// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <span>
#include <utility>
#include <vector>

namespace
{
using Point = sirkit::geo::Point<std::int64_t, std::int64_t>;
using MinkowskiPoint = std::pair<std::int64_t, std::int64_t>;

struct Hulls
{
    std::vector<Point> input;
    std::vector<Point> division;
};

std::vector<MinkowskiPoint> scale(std::span<const Point> points, std::int64_t factor)
{
    std::vector<MinkowskiPoint> result;
    result.reserve(points.size());
    for (const auto &p : points)
    {
        result.emplace_back(p.x * factor, p.y * factor);
    }
    return result;
}

Hulls division_hull(std::span<const Point> points, std::int64_t p, std::int64_t q)
{
    if (points.size() == 1)
    {
        return {{points.front()}, {}};
    }
    const auto middle = points.size() / 2;
    auto left = division_hull(points.first(middle), p, q);
    auto right = division_hull(points.subspan(middle), p, q);

    // 左から始まり右で終わる組の凸包を、倍率q・pのミンコフスキー和で求める。
    const auto crossing = sirkit::minkowski_sum(scale(left.input, q), scale(right.input, p));
    left.division.reserve(left.division.size() + right.division.size() + crossing.size());
    left.division.insert(left.division.end(), right.division.begin(), right.division.end());
    for (const auto &[x, y] : crossing)
    {
        left.division.push_back({x, y});
    }
    left.input.insert(left.input.end(), right.input.begin(), right.input.end());
    return {sirkit::geo::convex_hull(std::move(left.input)), sirkit::geo::convex_hull(std::move(left.division))};
}
} // namespace

int main()
{
    std::size_t n{};
    std::int64_t p{}, q{};
    verify_in >> n >> p >> q;
    std::vector<Point> points(n);
    for (auto &point : points)
    {
        verify_in >> point.x >> point.y;
    }
    const auto hull = division_hull(points, p, q).division;
    std::int64_t twice_area = 0;
    for (std::size_t i = 2; i < hull.size(); ++i)
    {
        twice_area += hull[0].orient(hull[i - 1], hull[i]);
    }
    const auto denominator = 2 * (p + q) * (p + q);
    const auto divisor = std::gcd(twice_area, denominator);
    verify_out << twice_area / divisor << ' ' << denominator / divisor << '\n';
}
