// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Disk_Lattice_Points.cpp"

#include <iostream>

int main()
{
    const auto points = sirkit::geo::count_disk_lattice_points(1, 0, 2, 2);
    std::cout << points.closed << ' ' << points.boundary << '\n'; // 2 0
}
