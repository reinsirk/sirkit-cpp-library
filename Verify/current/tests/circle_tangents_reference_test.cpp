// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_GEO_CIRCLE_TANGENTS_DEFINED
#include "../../../Geometry/Circle_Tangents.cpp"
#endif
#include <cassert>
#include <cmath>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>

namespace
{
using sirkit::geo::Circle;
using sirkit::geo::CircleTangent;
using sirkit::geo::RealPoint;

void CheckLine(const CircleTangent &line, Circle a, Circle b)
{
    assert(std::abs(line.normal.norm() - 1) < 1e-10L);
    assert(std::abs((line.first - a.center).norm() - a.radius) < 1e-9L);
    assert(std::abs((line.second - b.center).norm() - b.radius) < 1e-9L);
    assert(std::abs(line.normal.dot(line.second - line.first)) < 1e-9L);
}

std::size_t Count(int squared, int threshold)
{
    if (squared == threshold * threshold)
    {
        return 1;
    }
    return squared > threshold * threshold ? 2 : 0;
}

void Check(Circle a, int dx, int dy, int next_radius)
{
    const Circle b{a.center + RealPoint{static_cast<long double>(dx), static_cast<long double>(dy)},
                   static_cast<long double>(next_radius)};
    const int radius = static_cast<int>(a.radius);
    const int squared = dx * dx + dy * dy;
    const auto tangents = sirkit::geo::circle_tangents(a, b);
    if (squared == 0)
    {
        if (radius == next_radius)
        {
            assert(!tangents);
        }
        else
        {
            assert(tangents && tangents->empty());
        }
    }
    else
    {
        assert(tangents && tangents->size() == Count(squared, radius - next_radius) + Count(squared, radius + next_radius));
        for (const auto &line : *tangents)
        {
            CheckLine(line, a, b);
        }
        for (std::size_t i = 0; i < tangents->size(); ++i)
        {
            for (std::size_t j = i + 1; j < tangents->size(); ++j)
            {
                assert(((*tangents)[i].normal - (*tangents)[j].normal).norm() > 1e-9L);
            }
        }
    }
    const auto point = sirkit::geo::point_circle_tangents(a, b.center);
    assert(point.size() == Count(squared, radius));
    for (const auto &line : point)
    {
        assert(line.second == b.center);
        CheckLine(line, a, Circle{b.center, 0});
    }
}
} // namespace

int main()
{
    const Circle unit{{0, 0}, 1};
    const auto close_point = sirkit::geo::point_circle_tangents(unit, {1 - 1e-13L, 0});
    assert(close_point.size() == 1 && close_point[0].first == (RealPoint{1, 0}));
    assert(sirkit::geo::point_circle_tangents(unit, {1 - 1e-13L, 0}, {0, 0}).empty());
    assert(sirkit::geo::point_circle_tangents(unit, {1 + 1e-13L, 0}, {0, 0}).size() == 2);
    for (int r = 1; r <= 8; ++r)
    {
        for (int s = 1; s <= 8; ++s)
        {
            for (int dx = -18; dx <= 18; ++dx)
            {
                for (int dy = -4; dy <= 4; ++dy)
                {
                    Check({{17, -21}, static_cast<long double>(r)}, dx, dy, s);
                }
            }
        }
    }
    std::mt19937 rng(449235);
    for (int trial = 0; trial < 2500; ++trial)
    {
        Check({{-45, 144}, static_cast<long double>(rng() % 150 + 1)},
              static_cast<int>(rng() % 553) - 276, static_cast<int>(rng() % 711) - 355, static_cast<int>(rng() % 95 + 1));
    }
#ifndef SKIP_RUNTIME_CHECKS
    const auto inf = std::numeric_limits<long double>::infinity();
    for (const Circle &invalid : {Circle{{0, 0}, 0}, Circle{{0, 0}, -1}, Circle{{inf, 0}, 1}})
    {
        bool rejected = false;
        try
        {
            (void)sirkit::geo::circle_tangents(unit, invalid);
        }
        catch (const std::domain_error &)
        {
            rejected = true;
        }
        assert(rejected);
    }
    for (const RealPoint &point : {RealPoint{inf, 0}, RealPoint{0, inf}})
    {
        bool rejected = false;
        try
        {
            (void)sirkit::geo::point_circle_tangents(unit, point);
        }
        catch (const std::domain_error &)
        {
            rejected = true;
        }
        assert(rejected);
    }
#endif
}
