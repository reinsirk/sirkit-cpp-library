// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/Orthogonal_Polygon_Coverage.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <vector>

using sirkit::geo::orthogonal_polygon_coverage;
using sirkit::geo::OrthogonalPoint;
using sirkit::geo::OrthogonalPolygon;

bool ContainsCell(const OrthogonalPolygon &polygon, const OrthogonalPoint &cell)
{
    bool inside = false;
    for (std::size_t i = 0; i < polygon.size(); ++i)
    {
        const auto &[x1, y1] = polygon[i];
        const auto &[x2, y2] = polygon[(i + 1) % polygon.size()];
        if (x1 == x2 && x1 > cell.first && ((y1 > cell.second) != (y2 > cell.second)))
        {
            inside = !inside;
        }
    }
    return inside;
}

int main()
{
    const std::array<OrthogonalPoint, 3> empty_cells{{{0, 0}, {1, 1}, {2, 2}}};
    assert((orthogonal_polygon_coverage({}, empty_cells) == std::vector<std::int64_t>{0, 0, 0}));
    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    const std::array<OrthogonalPolygon, 1> extremes{{{{kMin, kMin}, {kMax, kMin}, {kMax, kMax}, {kMin, kMax}}}};
    const std::array<OrthogonalPoint, 4> extreme_cells{{{kMin, kMin}, {kMax - 1, kMax - 1}, {kMax, kMin}, {kMin, kMax}}};
    assert((orthogonal_polygon_coverage(extremes, extreme_cells) == std::vector<std::int64_t>{1, 1, 0, 0}));
    assert(orthogonal_polygon_coverage(extremes, {}).empty());

    const std::vector<OrthogonalPolygon> separate{
        {{0, -50}, {4, -50}, {4, -40}, {0, -40}},
        {{1, 0}, {3, 0}, {3, 1}, {1, 1}},
    };
    const std::array<OrthogonalPoint, 1> one_cell{{{2, 0}}};
    assert(orthogonal_polygon_coverage(separate, one_cell).front() == 1);

    std::mt19937 random(26045);
    for (int trial = 0; trial < 180; ++trial)
    {
        std::vector<OrthogonalPolygon> polygons;
        for (int i = 0; i < 1 + trial % 29; ++i)
        {
            const auto x = static_cast<int>(random() % 23) - 11;
            const auto y = static_cast<int>(random() % 17) - 8;
            const auto width = 2 + random() % 9;
            const auto height = 2 + random() % 7;
            const auto a = x + static_cast<int>(width / 2);
            const auto b = y + static_cast<int>(height / 2);
            const auto r = x + static_cast<int>(width), t = y + static_cast<int>(height);
            OrthogonalPolygon p{{x, y}, {r, y}, {r, b}, {a, b}, {a, t}, {x, t}};
            if (random() % 2)
            {
                std::reverse(p.begin(), p.end());
            }
            std::rotate(p.begin(), p.begin() + random() % p.size(), p.end());
            if (random() % 2)
            {
                p.push_back(p.front());
            }
            polygons.push_back(std::move(p));
        }
        std::vector<OrthogonalPoint> cells;
        for (int j = 0; j < 130; ++j)
        {
            const auto x = static_cast<int>(random() % 42) - 13;
            const auto y = static_cast<int>(random() % 40) - 13;
            cells.emplace_back(x, y);
        }
        const auto actual = orthogonal_polygon_coverage(polygons, cells);
        for (std::size_t j = 0; j < cells.size(); ++j)
        {
            const auto expected = std::count_if(polygons.begin(), polygons.end(),
                                                [&](const auto &polygon)
                                                {
                                                    return ContainsCell(polygon, cells[j]);
                                                });
            assert(actual[j] == expected);
        }
    }

#ifndef SKIP_RUNTIME_CHECKS
    const auto invalid = [&](OrthogonalPolygon polygon)
    {
        try
        {
            const std::array<OrthogonalPolygon, 1> bad{polygon};
            (void)orthogonal_polygon_coverage(bad, empty_cells);
        }
        catch (const std::invalid_argument &)
        {
            return true;
        }
        return false;
    };
    assert(invalid({{0, 0}}));
    assert(invalid({{0, 0}, {2, 2}, {2, 1}, {0, 1}}));
    assert(invalid({{0, 0}, {0, 0}, {2, 0}, {2, 1}, {0, 1}}));
#endif
}
