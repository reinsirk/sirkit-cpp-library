// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_CONVEX_LAYERS_DEFINED
#include "../../../Geometry/Convex_Layers.cpp"
#endif
#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Point = std::pair<std::int64_t, std::int64_t>;
using Points = std::vector<Point>;

std::vector<std::size_t> reference_layers(const Points &points)
{
    std::vector<std::size_t> layer(points.size());
    for (std::size_t current = 1; std::ranges::find(layer, 0) != layer.end(); ++current)
    {
        // 任意の2点を通る直線を試し、残りの点が片側にしかなければその両点は境界上にある。
        std::vector<bool> boundary(points.size());
        bool distinct = false;
        for (std::size_t i = 0; i < points.size(); ++i)
        {
            if (layer[i] != 0)
            {
                continue;
            }
            for (std::size_t j = 0; j < i; ++j)
            {
                if (layer[j] != 0 || points[i] == points[j])
                {
                    continue;
                }
                distinct = true;
                const auto [u, v] = points[i];
                const auto [x, y] = points[j];
                bool positive = false, negative = false;
                for (std::size_t k = 0; k < points.size(); ++k)
                {
                    if (layer[k] == 0)
                    {
                        const auto [a, b] = points[k];
                        const auto side = (x - u) * (b - v) - (y - v) * (a - u);
                        positive |= side > 0;
                        negative |= side < 0;
                    }
                }
                if (!positive || !negative)
                {
                    boundary[i] = true;
                    boundary[j] = true;
                }
            }
        }
        for (std::size_t i = 0; i < points.size(); ++i)
        {
            if (layer[i] == 0 && (boundary[i] || !distinct))
            {
                layer[i] = current;
            }
        }
    }
    return layer;
}

void compare(const Points &points)
{
    assert(sirkit::convex_layers(points) == reference_layers(points));
}
} // namespace

int main()
{
    compare({});
    compare({{0, 0}});
    compare({{1, 1}, {1, 1}, {1, 1}});
    compare({{0, 0}, {0, 1}, {0, 2}, {1, 1}, {2, 1}, {3, 1}});
    compare({{4, 4}, {4, 0}, {0, 4}, {0, 0}, {2, 0}, {4, 2}, {2, 4}, {0, 2}, {2, 2}});
    compare({{0, 0}, {1, 2}, {0, 2}, {1, 1}, {0, 1}, {0, 0}, {0, 1}, {1, 0}});
    constexpr auto hi = std::numeric_limits<std::int64_t>::max();
    constexpr auto lo = std::numeric_limits<std::int64_t>::min();
    compare({{hi, lo}, {hi, lo + 1'000'000}, {hi - 1'000'000, lo}, {hi - 1'000'000, lo + 1'000'000}, {hi - 500'000, lo + 500'000}});

    bool threw = false;
    try
    {
        (void)sirkit::convex_layers(Points{{lo, 0}, {hi, 0}});
    }
    catch (const std::overflow_error &)
    {
        threw = true;
    }
    assert(threw);

    for (unsigned mask = 0; mask < (1U << 9); ++mask)
    {
        Points points;
        for (int i = 0; i < 9; ++i)
        {
            if (mask & (1U << i))
            {
                points.emplace_back(i / 3, i % 3);
            }
        }
        compare(points);
    }

    std::mt19937 random(6742);
    for (int trial = 0; trial < 5000; ++trial)
    {
        Points points(random() % 39);
        const unsigned scale_x = trial % 4 == 0 ? 3 : (trial % 4 == 1 ? 49 : 1'000'001);
        const unsigned scale_y = trial % 5 == 0 ? 3 : (trial % 5 == 1 ? 49 : 1'000'001);
        for (auto &[x, y] : points)
        {
            x = random() % scale_x;
            y = random() % scale_y;
        }
        compare(points);
    }

    Points diamonds, vertical;
    std::vector<std::size_t> expected;
    for (std::size_t depth = 0; depth < 50'000; ++depth)
    {
        const auto radius = static_cast<std::int64_t>(50'000 - depth);
        diamonds.insert(diamonds.end(), {{radius, 0}, {0, radius}, {-radius, 0}, {0, -radius}});
        expected.insert(expected.end(), 4, depth + 1);
    }
    assert(sirkit::convex_layers(diamonds) == expected);
    for (std::int64_t i = 0; i < 200'000; ++i)
    {
        vertical.emplace_back(9, i);
    }
    assert(sirkit::convex_layers(vertical) == std::vector<std::size_t>(vertical.size(), 1));
}
