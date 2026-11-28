// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Half_Plane_Intersection.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array planes{
        sirkit::HalfPlane{-1, 0, 0},
        sirkit::HalfPlane{0, -1, 0},
        sirkit::HalfPlane{1, 1, 3},
    };
    const auto triangle = sirkit::half_plane_intersection(planes, {-10, -10}, {10, 10});
    assert(triangle.size() == 3);
}
