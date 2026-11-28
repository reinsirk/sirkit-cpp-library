// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/Convex_Polygon_Locator.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

using Point = sirkit::geo::Point<std::int64_t, std::int64_t>;
using Locator = sirkit::geo::ConvexPolygonLocator<std::int64_t, std::int64_t>;
using sirkit::geo::Location;

std::int64_t Determinant(Point a, Point b, Point c)
{
    return (b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x);
}

std::vector<Point> Hull(std::vector<Point> points)
{
    std::sort(points.begin(), points.end());
    points.erase(std::unique(points.begin(), points.end()), points.end());
    if (points.size() < 2)
    {
        return points;
    }
    std::vector<Point> hull;
    auto append_chain = [&](auto first, auto last)
    {
        const auto base = hull.size();
        while (first != last)
        {
            while (hull.size() >= base + 2 && Determinant(hull[hull.size() - 2], hull.back(), *first) <= 0)
            {
                hull.pop_back();
            }
            hull.push_back(*first++);
        }
        hull.pop_back();
    };
    append_chain(points.begin(), points.end());
    append_chain(points.rbegin(), points.rend());
    return hull;
}

Location Linear(const std::vector<Point> &hull, Point point)
{
    bool boundary = false;
    for (std::size_t i = 0; i < hull.size(); ++i)
    {
        const Point a = hull[i], b = hull[(i + 1) % hull.size()];
        const auto side = Determinant(a, b, point);
        if (side < 0)
        {
            return Location::Outside;
        }
        if (side == 0 && std::min(a, b) <= point && point <= std::max(a, b))
        {
            boundary = true;
        }
    }
    if (boundary)
    {
        return Location::Boundary;
    }
    return hull.size() >= 3 ? Location::Inside : Location::Outside;
}

int main()
{
    Locator empty;
    assert(empty.locate({0, 0}) == Location::Outside);
    const Locator one({{2, 3}, {2, 3}, {2, 3}});
    assert(one.locate({2, 3}) == Location::Boundary && one.locate({2, 4}) == Location::Outside);
    const Locator line({{4, 5}, {3, 4}, {2, 3}, {3, 4}, {4, 5}});
    assert(line.locate({3, 4}) == Location::Boundary && line.locate({4, 5}) == Location::Boundary);
    assert(line.locate({1, 2}) == Location::Outside);
    assert(line.locate({5, 6}) == Location::Outside && line.locate({2, 4}) == Location::Outside);
    const Locator square({{-1000000000, -1000000000}, {1000000000, -1000000000}, {1000000000, 1000000000}, {-1000000000, 1000000000}});
    assert(square.locate({0, 0}) == Location::Inside);
    assert(square.locate({1000000000, 1000000000}) == Location::Boundary);

    std::mt19937 random(152921);
    for (int trial = 0; trial < 450; ++trial)
    {
        std::vector<Point> points(20 + random() % 120);
        for (auto &point : points)
        {
            point.x = 2 * (static_cast<int>(random() % 201) - 100);
            point.y = 2 * (static_cast<int>(random() % 201) - 100);
        }
        const auto hull = Hull(std::move(points));
        std::vector<Point> input;
        for (std::size_t i = 0; i < hull.size(); ++i)
        {
            const Point a = hull[i], b = hull[(i + 1) % hull.size()];
            input.push_back(a);
            input.push_back(a);
            input.push_back({(a.x + b.x) / 2, (a.y + b.y) / 2});
        }
        if (trial % 2)
        {
            std::reverse(input.begin(), input.end());
        }
        std::rotate(input.begin(), input.begin() + random() % input.size(), input.end());
        input.push_back(input.front());
        const Locator locator(std::move(input));
        for (std::size_t i = 0; i < hull.size(); ++i)
        {
            const Point a = hull[i], b = hull[(i + 1) % hull.size()];
            assert(locator.locate(a) == Location::Boundary);
            const Point midpoint{(a.x + b.x) / 2, (a.y + b.y) / 2};
            assert(locator.locate(midpoint) == Location::Boundary);
        }
        for (int i = 0; i < 100; ++i)
        {
            const Point point{static_cast<int>(random() % 481) - 240, static_cast<int>(random() % 481) - 240};
            assert(locator.locate(point) == Linear(hull, point));
        }
    }
}
