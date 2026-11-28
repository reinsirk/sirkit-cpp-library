// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/Manhattan_Nearest_Neighbors.cpp"

#include <cassert>
#include <cstdlib>
#include <random>

namespace
{
using Point = sirkit::geo::ManhattanPoint;

void check(const std::vector<Point> &points)
{
    const auto answer = sirkit::geo::manhattan_nearest_neighbors(points);
    assert(answer.size() == points.size());
    for (std::size_t i = 0; i < points.size(); ++i)
    {
        std::uint64_t best = std::numeric_limits<std::uint64_t>::max();
        std::size_t at = points.size();
        for (std::size_t j = 0; j < points.size(); ++j)
        {
            if (i == j)
            {
                continue;
            }
            const auto x = static_cast<std::int64_t>(points[i][0]) - points[j][0];
            const auto y = static_cast<std::int64_t>(points[i][1]) - points[j][1];
            const auto distance = static_cast<std::uint64_t>(std::abs(x) + std::abs(y));
            if (distance < best)
            {
                best = distance;
                at = j;
            }
        }
        if (at == points.size())
        {
            assert(!answer[i]);
        }
        else
        {
            assert(answer[i] && answer[i]->distance == best && answer[i]->index == at);
        }
    }
}
} // namespace

int main()
{
    const auto lo = std::numeric_limits<std::int32_t>::min();
    const auto hi = std::numeric_limits<std::int32_t>::max();
    check({});
    check({{hi, lo}});
    check({{hi, lo}, {lo, hi}});
    check({{lo, hi}, {0, 0}, {lo, hi}, {0, 0}, {lo, hi}, {1, 1}});
    check({{0, -4}, {0, 12}, {0, 2}, {0, -3}});
    std::mt19937 gen(367817);
    for (int trial = 0; trial < 600; ++trial)
    {
        std::vector<Point> points(gen() % 54);
        for (auto &[x, y] : points)
        {
            x = static_cast<std::int32_t>(gen() % 21) - 10;
            y = static_cast<std::int32_t>(gen() % 21) - 10;
        }
        check(points);
    }
}
