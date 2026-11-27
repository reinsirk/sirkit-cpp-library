// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Geometry/Minimum_Enclosing_Circle.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/minimum_enclosing_circle.md
//
// API一覧
// minimum_enclosing_circle(points,seed=0): 整数点列を含む最小円。空ならnullopt。
// 各座標は[-10000,10000]。重複可。入力を変えない。乱択順で期待O(N)、最悪O(N³)時間・O(N)領域。
// MinimumEnclosingCircle::center()/radius_squared(): 中心・半径二乗のdouble近似。O(1)。
// MinimumEnclosingCircle::contains(p)/on_boundary(p): 同じ範囲の整数点を整数演算で厳密に判定。O(1)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MINIMUM_ENCLOSING_CIRCLE_DEFINED
#define SIRKIT_MINIMUM_ENCLOSING_CIRCLE_DEFINED

class MinimumEnclosingCircle
{
public:
    using Point = std::pair<std::int32_t, std::int32_t>;

    [[nodiscard]] std::pair<double, double> center() const noexcept
    {
        const auto [x, y] = offset();
        return {origin_.first + x, origin_.second + y};
    }

    [[nodiscard]] double radius_squared() const noexcept
    {
        const auto [x, y] = offset();
        return x * x + y * y;
    }

    [[nodiscard]] bool contains(Point point) const
    {
        check_coordinates(point);
        return power(point) <= 0;
    }

    [[nodiscard]] bool on_boundary(Point point) const
    {
        check_coordinates(point);
        return power(point) == 0;
    }

private:
    Point origin_;
    std::int64_t x_, y_, denominator_;

    MinimumEnclosingCircle(Point origin, std::int64_t x, std::int64_t y, std::int64_t denominator) noexcept
        : origin_(origin), x_(x), y_(y), denominator_(denominator)
    {
    }

    static void check_coordinates([[maybe_unused]] Point point)
    {
    }

    [[nodiscard]] std::pair<double, double> offset() const noexcept
    {
        const auto denominator = static_cast<double>(denominator_);
        return {static_cast<double>(x_) / denominator, static_cast<double>(y_) / denominator};
    }

    [[nodiscard]] std::int64_t power(Point point) const noexcept
    {
        const auto dx = static_cast<std::int64_t>(point.first) - origin_.first;
        const auto dy = static_cast<std::int64_t>(point.second) - origin_.second;
        // origin_ は円周上。円の外・上・内を示す符号を、分母を掛けて求める。
        // 座標差は20000以下なので、全体の絶対値は24×20000^4以下。
        return denominator_ * (dx * dx + dy * dy) - 2 * (x_ * dx + y_ * dy);
    }

    static MinimumEnclosingCircle with_diameter(Point a, Point b) noexcept
    {
        return {a, static_cast<std::int64_t>(b.first) - a.first,
                static_cast<std::int64_t>(b.second) - a.second, 2};
    }

    static MinimumEnclosingCircle through_three(Point a, Point b, Point c) noexcept
    {
        const auto bx = static_cast<std::int64_t>(b.first) - a.first;
        const auto by = static_cast<std::int64_t>(b.second) - a.second;
        const auto cx = static_cast<std::int64_t>(c.first) - a.first;
        const auto cy = static_cast<std::int64_t>(c.second) - a.second;
        const auto b2 = bx * bx + by * by;
        const auto c2 = cx * cx + cy * cy;
        auto denominator = 2 * (bx * cy - by * cx);
        auto x = b2 * cy - c2 * by;
        auto y = bx * c2 - cx * b2;
        if (denominator < 0)
        {
            denominator = -denominator;
            x = -x;
            y = -y;
        }
        return {a, x, y, denominator};
    }

    friend std::optional<MinimumEnclosingCircle> minimum_enclosing_circle(std::span<const Point> points,
                                                                          std::uint64_t seed);
};

inline std::optional<MinimumEnclosingCircle> minimum_enclosing_circle(
    std::span<const MinimumEnclosingCircle::Point> points, std::uint64_t seed = 0)
{
    if (points.empty())
    {
        return std::nullopt;
    }
    using Circle = MinimumEnclosingCircle;
    std::vector<Circle::Point> shuffled;
    shuffled.reserve(points.size());
    for (const auto &point : points)
    {
        Circle::check_coordinates(point);
        shuffled.push_back(point);
    }
    std::mt19937_64 random(seed);
    std::ranges::shuffle(shuffled, random);

    auto circle = Circle::with_diameter(shuffled.front(), shuffled.front());
    for (std::size_t i = 1; i < shuffled.size(); ++i)
    {
        const auto a = shuffled[i];
        if (circle.power(a) <= 0)
        {
            continue;
        }
        circle = Circle::with_diameter(a, a);
        for (std::size_t j = 0; j < i; ++j)
        {
            const auto b = shuffled[j];
            if (circle.power(b) <= 0)
            {
                continue;
            }
            circle = Circle::with_diameter(a, b);
            for (std::size_t k = 0; k < j; ++k)
            {
                const auto c = shuffled[k];
                if (circle.power(c) > 0)
                {
                    // a,b を円周に置く制約には解がある。同一直線上で既存の円の外に
                    // ある点は、その制約のもとでは含められないので、この分岐に来ない。
                    circle = Circle::through_three(a, b, c);
                }
            }
        }
    }
    return circle;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
