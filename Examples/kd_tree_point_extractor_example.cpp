// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Kd_Tree_Point_Extractor.cpp"
#include <algorithm>
#include <iostream>

int main()
{
    sirkit::KdTreePointExtractor points({{0, 1}, {2, 2}, {0, 1}, {5, 5}});
    std::vector<std::size_t> ids;
    points.take_in_circle({0, 0}, 1, ids);
    std::sort(ids.begin(), ids.end());
    for (const auto id : ids)
    {
        std::cout << id << ' ';
    }
    std::cout << '\n';                  // 0 2
    std::cout << points.size() << '\n'; // 2
}
