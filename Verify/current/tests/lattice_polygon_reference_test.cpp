// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/Lattice_Polygon.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Vertex = std::pair<std::int64_t, std::int64_t>;

int classify(const std::vector<Vertex> &polygon, Vertex point)
{
    bool inside = false;
    const auto [x, y] = point;
    auto a = polygon.back();
    for (Vertex b : polygon)
    {
        const auto cross = (b.first - a.first) * (y - a.second) - (b.second - a.second) * (x - a.first);
        if (cross == 0 && std::min(a.first, b.first) <= x && x <= std::max(a.first, b.first) &&
            std::min(a.second, b.second) <= y && y <= std::max(a.second, b.second))
        {
            return 1;
        }
        if ((a.second > y) != (b.second > y) && ((cross > 0) == (b.second > a.second)))
        {
            inside = !inside;
        }
        a = b;
    }
    return inside ? 2 : 0;
}

void check(const std::vector<Vertex> &points, const sirkit::LatticePolygonPoints expected)
{
    const auto result = sirkit::lattice_polygon_points(points);
    assert(result.interior == expected.interior && result.boundary == expected.boundary);
}

template <class Exception, class Call>
void must_throw(Call call)
{
    bool caught = false;
    try
    {
        call();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}
}

int main()
{
    check({}, {0, 0});
    check({{0, 1}}, {0, 1});
    check({{3, 5}, {-3, -7}, {0, -1}, {-2, -5}}, {0, 7});
    check({{0, 0}, {4, 0}, {2, 3}, {0, 0}}, {4, 6});
    must_throw<std::overflow_error>([]
                                    {
                                        check({{0, 0}, {std::numeric_limits<std::int64_t>::max(), 0}}, {});
                                    });

    constexpr auto kWidth = std::uint64_t{0xffffffff};
    constexpr auto kBaseX = std::numeric_limits<std::int64_t>::min();
    constexpr auto kBaseY = std::numeric_limits<std::int64_t>::max() - static_cast<std::int64_t>(kWidth);
    constexpr auto kEndX = kBaseX + static_cast<std::int64_t>(kWidth);
    constexpr auto kEndY = std::numeric_limits<std::int64_t>::max();
    std::vector<Vertex> full_square{{kBaseX, kBaseY}, {kEndX, kBaseY}, {kEndX, kEndY}, {kBaseX, kEndY}};
    const sirkit::LatticePolygonPoints expected{(kWidth - 1) * (kWidth - 1), 4 * kWidth};
    check(full_square, expected);
    std::ranges::reverse(full_square);
    check(full_square, expected);
    must_throw<std::overflow_error>([&]
                                    {
                                        static_cast<void>(sirkit::lattice_polygon_points(full_square).total());
                                    });
    assert(sirkit::lattice_polygon_points(std::array{Vertex{1, 3}, Vertex{1, 4}}).total() == 2);

    sirkit::detail::LatticePolygonDoubleArea negative_multiple{std::numeric_limits<std::uint64_t>::max(), 0};
    negative_multiple.make_absolute();
    assert(negative_multiple.high == 1 && negative_multiple.low == 0);
    must_throw<std::invalid_argument>([]
                                      {
                                          static_cast<void>(sirkit::detail::LatticePolygonDoubleArea{}.count_interior(3));
                                      });
    must_throw<std::overflow_error>([]
                                    {
                                        static_cast<void>(sirkit::detail::LatticePolygonDoubleArea{2, 0}.count_interior(1));
                                    });

    std::mt19937 random(763941);
    for (int trial = 0; trial < 450; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 12);
        int height = 1 + static_cast<int>(random() % 9);
        std::vector<Vertex> polygon{{0, 0}, {n, 0}, {n, height}};
        for (int x = n - 1; x >= 0; --x)
        {
            polygon.emplace_back(x, height);
            if (x > 0)
            {
                height = 1 + static_cast<int>(random() % 9);
                polygon.emplace_back(x, height);
            }
        }
        if (trial % 2 == 0)
        {
            std::ranges::reverse(polygon);
        }
        sirkit::LatticePolygonPoints exact{};
        for (int x = 0; x <= n; ++x)
        {
            for (int y = 0; y <= 9; ++y)
            {
                const int position = classify(polygon, {x, y});
                exact.boundary += position == 1;
                exact.interior += position == 2;
            }
        }
        check(polygon, exact);
    }
}
