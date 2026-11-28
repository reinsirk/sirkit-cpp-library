// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Manhattan_Nearest_Neighbors.cpp"
#include <iostream>

int main()
{
    const std::vector<sirkit::geo::ManhattanPoint> points{{0, 0}, {3, 1}, {2, 1}, {0, 0}};
    for (const auto &item : sirkit::geo::manhattan_nearest_neighbors(points))
    {
        std::cout << item->distance << ' ' << item->index << '\n';
    }
    // 0 3
    // 1 2
    // 1 1
    // 0 0
}
