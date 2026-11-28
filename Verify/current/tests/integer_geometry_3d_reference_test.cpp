// SPDX-License-Identifier: CC0-1.0
#include "Geometry/Integer_Geometry_3D.cpp"

#include <array>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <random>
#include <stdexcept>

namespace
{
using sirkit::Point3i;
using sirkit::PrismPosition;

Point3i diff(Point3i x, Point3i y)
{
    return {x.x - y.x, x.y - y.y, x.z - y.z};
}

std::int64_t dot(Point3i x, Point3i y)
{
    return x.x * y.x + x.y * y.y + x.z * y.z;
}

PrismPosition gram_prism(Point3i a, Point3i b, Point3i c, Point3i p)
{
    const auto u = diff(b, a), v = diff(c, a), w = diff(p, a);
    const auto uu = dot(u, u), vv = dot(v, v), uv = dot(u, v);
    const auto determinant = uu * vv - uv * uv;
    if (determinant == 0)
    {
        return PrismPosition::kDegenerate;
    }
    const auto first = dot(w, u) * vv - dot(w, v) * uv;
    const auto second = dot(w, v) * uu - dot(w, u) * uv;
    if (first < 0 || second < 0 || first + second > determinant)
    {
        return PrismPosition::kOutside;
    }
    return first == 0 || second == 0 || first + second == determinant ? PrismPosition::kBoundary : PrismPosition::kInside;
}
}

int main()
{
    using sirkit::orientation_3d;
    using sirkit::signed_plane_distance;
    using sirkit::triangular_prism_position;
    const Point3i a{0, 0, 0}, b{2, 0, 0}, c{0, 6, 0};
    const Point3i above{1, 1, 20}, below{1, 0, -5};
    assert(orientation_3d(a, b, c, above) == 1);
    assert(orientation_3d(a, b, c, below) == -1);
    assert(orientation_3d(a, b, c, c) == 0);
    assert(signed_plane_distance(a, b, c, above) == 20);
    assert(signed_plane_distance(a, b, c, below) == -5);
    assert(signed_plane_distance(a, c, b, below) == 5);
    assert(!signed_plane_distance(a, b, b, below));
    assert(triangular_prism_position(a, b, b, below) == PrismPosition::kDegenerate);
    assert(triangular_prism_position(a, b, c, above) == PrismPosition::kInside);
    assert(triangular_prism_position(a, b, c, below) == PrismPosition::kBoundary);
    assert(triangular_prism_position(a, b, c, {2, 1, -5}) == PrismPosition::kOutside);
    assert(triangular_prism_position(a, c, b, above) == PrismPosition::kInside);

    std::mt19937 rng(236773);
    const auto sample = [&]
    {
        return static_cast<int>(rng() % 41) - 20;
    };
    for (int trial = 0; trial < 2200; ++trial)
    {
        std::array<Point3i, 4> points;
        for (auto &point : points)
        {
            point = {sample(), sample(), sample()};
        }
        auto [x, y, z, p] = points;
        const auto expected = gram_prism(x, y, z, p);
        assert(triangular_prism_position(x, y, z, p) == expected);
        assert(triangular_prism_position(y, x, z, p) == expected);
        assert(triangular_prism_position(x, z, y, x) == gram_prism(x, z, y, x));
    }

    constexpr std::int64_t kLim = 1000000000;
    const Point3i v{-kLim, -kLim, -kLim}, w{kLim, -kLim, kLim}, z{-kLim, kLim, kLim};
    assert(triangular_prism_position(v, w, z, {0, 0, 0}) == PrismPosition::kInside);
    assert(triangular_prism_position(v, w, z, {kLim, kLim, kLim}) == PrismPosition::kOutside);
    assert(std::abs(*signed_plane_distance(v, w, z, {0, 0, 0}) + kLim / std::sqrt(3.0)) < 1e-6);
    for (const auto outside : {Point3i{-kLim - 1, 0, 0}, Point3i{0, 0, kLim + 1}})
    {
        try
        {
            (void)orientation_3d(a, b, c, outside);
            assert(false);
        }
        catch (const std::out_of_range &)
        {
        }
    }
}
