// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Prefix_Sum_ND.cpp"
#include <iostream>

int main()
{
    sirkit::CumulativeSum2D<int> grid(3, 4);
    grid.add(0, 1, 5);
    grid.add(2, 3, 7);
    grid.build();
    std::cout << grid.query(0, 1, 3, 4) << '\n'; // 12

    sirkit::CumulativeSum3D<int> cube(2, 2, 2);
    cube.imos(0, 0, 0, 2, 2, 2, 3);
    cube.build();
    std::cout << cube.imos_get(1, 1, 1) << '\n'; // 3
}
