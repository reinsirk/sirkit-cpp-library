// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Dominance_Counts.cpp"

#include <cassert>
#include <cstddef>
#include <functional>
#include <limits>
#include <random>
#include <string>
#include <utility>
#include <vector>

using Point = std::pair<int, int>;
using sirkit::dominance_counts;

template <typename P, typename XLess, typename YLess>
void compare_with_quadratic(const std::vector<P> &points, XLess x_less, YLess y_less)
{
    for (const bool strict_x : {false, true})
    {
        for (const bool strict_y : {false, true})
        {
            const auto counts = dominance_counts(points, strict_x, strict_y, x_less, y_less);
            assert(counts.size() == points.size());
            for (std::size_t i = 0; i < points.size(); ++i)
            {
                int expected = 0;
                for (const auto &other : points)
                {
                    const auto &point = points[i];
                    const bool x_included = strict_x ? x_less(other.first, point.first) : !x_less(point.first, other.first);
                    const bool y_included = strict_y ? y_less(other.second, point.second) : !y_less(point.second, other.second);
                    expected += x_included && y_included;
                }
                assert(counts[i] == expected);
            }
        }
    }
}

void edge_cases()
{
    assert(dominance_counts(std::vector<Point>{}).empty());
    const std::vector<Point> single{{3, 2}};
    assert(dominance_counts(single) == std::vector<int>{1});
    assert(dominance_counts(single, true) == std::vector<int>{0});
    compare_with_quadratic(std::vector<Point>(18, Point{-17, 23}), std::less<>{}, std::greater<>{});

    constexpr auto low = std::numeric_limits<int>::min();
    constexpr auto high = std::numeric_limits<int>::max();
    const std::vector<Point> extremes{{low, low}, {high, low}, {low, high}, {high, high}, {0, 0}};
    compare_with_quadratic(extremes, std::less<>{}, std::less<>{});
    compare_with_quadratic(extremes, std::greater<>{}, std::greater<>{});

    const std::vector<std::pair<std::string, int>> labels{{"aa", 12}, {"c", 11}, {"b", 3}, {"aa", 18}};
    const auto by_tens = [](int a, int b)
    {
        return a / 10 < b / 10;
    };
    compare_with_quadratic(labels, std::less<>{}, by_tens);
}

void random_cases()
{
    std::mt19937 random(0xa436);
    for (int test = 0; test < 1700; ++test)
    {
        std::vector<Point> points(random() % 45);
        for (auto &[x, y] : points)
        {
            x = static_cast<int>(random() % 19) - 9;
            y = static_cast<int>(random() % 17) - 8;
        }
        compare_with_quadratic(points, std::less<>{}, std::less<>{});
        compare_with_quadratic(points, std::greater<>{}, std::less<>{});
        compare_with_quadratic(points, std::less<>{}, std::greater<>{});
    }
}

int main()
{
    edge_cases();
    random_cases();
}
