// SPDX-License-Identifier: CC0-1.0
#include <cstddef>
#include <iostream>
#include <vector>

int main()
{
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::size_t n = 0;
    std::cin >> n;
    std::vector<DelaunayPoint> points(n);
    for (auto &[x, y] : points)
    {
        std::cin >> x >> y;
    }
    std::cout << delaunay_triangulation(points).size() << '\n';
}
