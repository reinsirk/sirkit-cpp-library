// SPDX-License-Identifier: CC0-1.0
#include "../../../Geometry/Lattice_Shape_Transforms.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <ranges>
#include <span>
#include <utility>
#include <vector>

using Point = std::pair<std::int64_t, std::int64_t>;
using Orientation = sirkit::LatticeOrientation;
using sirkit::lattice_shape_transforms;

Point reorient(Point point, int orientation)
{
    if (orientation >= 4)
    {
        point.first = -point.first;
    }
    for (int quarter = 0; quarter < orientation % 4; ++quarter)
    {
        point = {-point.second, point.first};
    }
    return point;
}

void normalize(std::vector<Point> &points)
{
    if (points.empty())
    {
        return;
    }
    const auto min_x = std::ranges::min(points | std::views::keys);
    const auto min_y = std::ranges::min(points | std::views::values);
    for (auto &[x, y] : points)
    {
        x -= min_x;
        y -= min_y;
    }
    std::ranges::sort(points);
}

std::vector<Orientation> brute(std::span<const Point> source, std::span<const Point> target, bool reflect)
{
    std::vector<Orientation> result;
    std::vector<Point> goal(target.begin(), target.end());
    normalize(goal);
    for (int orientation = 0; orientation < (reflect ? 8 : 4); ++orientation)
    {
        std::vector<Point> viewed;
        for (const auto &point : source)
        {
            viewed.push_back(reorient(point, orientation));
        }
        normalize(viewed);
        if (viewed == goal)
        {
            result.push_back(static_cast<Orientation>(orientation));
        }
    }
    return result;
}

void boundaries_and_chirality()
{
    assert(lattice_shape_transforms({}, {}).size() == 4);
    assert(lattice_shape_transforms({}, {}, true).size() == 8);
    const std::array single{Point{30, -7}};
    assert(lattice_shape_transforms(single, {}).empty());
    assert(lattice_shape_transforms(single, single, true).size() == 8);

    const std::array source{Point{0, 0}, Point{1, 0}, Point{0, 1}, Point{0, 2}};
    const std::array reflected{Point{0, 0}, Point{-1, 0}, Point{0, 1}, Point{0, 2}};
    assert(lattice_shape_transforms(source, reflected).empty());
    assert(lattice_shape_transforms(source, reflected, true) == std::vector{Orientation::kReflect});

    using Limit = std::numeric_limits<std::int64_t>;
    const std::array wide{Point{Limit::min(), 0}, Point{Limit::max(), 1}};
    const std::array tall{Point{1, Limit::min()}, Point{0, Limit::max()}};
    const std::vector matches{Orientation::kRotate90, Orientation::kRotate270};
    assert(lattice_shape_transforms(wide, tall) == matches);

    const std::array repeated{Point{0, 0}, Point{0, 0}, Point{1, 0}, Point{0, 1}};
    const std::array distinct{Point{0, 0}, Point{1, 1}, Point{1, 0}, Point{0, 1}};
    assert(lattice_shape_transforms(repeated, distinct, true).empty());
}

void randomized_cases()
{
    std::mt19937 random(0xcc2ba3);
    for (int test = 0; test < 1700; ++test)
    {
        std::vector<Point> source(random() % 34);
        for (auto &[x, y] : source)
        {
            x = static_cast<std::int64_t>(random() % 15) - 7;
            y = static_cast<std::int64_t>(random() % 21) - 10;
        }
        const int orientation = static_cast<int>(random() % 8);
        std::vector<Point> target;
        for (const auto &point : source)
        {
            const auto [x, y] = reorient(point, orientation);
            target.emplace_back(x - 29, y + 56);
        }
        std::ranges::shuffle(target, random);
        if (test % 5 == 0 && !target.empty())
        {
            target.front().first += 2;
        }
        if (test % 11 == 0)
        {
            target.emplace_back(0, 0);
        }
        assert(lattice_shape_transforms(source, target) == brute(source, target, false));
        assert(lattice_shape_transforms(source, target, true) == brute(source, target, true));
    }
}

int main()
{
    boundaries_and_chirality();
    randomized_cases();
}
