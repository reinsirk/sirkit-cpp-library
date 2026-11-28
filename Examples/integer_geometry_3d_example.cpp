// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Integer_Geometry_3D.cpp"

#include <cassert>

int main()
{
    const sirkit::Point3i a{0, 0, 0}, b{3, 0, 0}, c{0, 3, 0}, p{1, 1, 2};
    assert(sirkit::orientation_3d(a, b, c, p) == 1);
    assert(sirkit::triangular_prism_position(a, b, c, p) == sirkit::PrismPosition::kInside);
    assert(sirkit::signed_plane_distance(a, b, c, p) == 2);
}
