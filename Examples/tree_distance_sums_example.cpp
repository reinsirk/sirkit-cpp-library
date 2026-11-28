// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Distance_Sums.cpp"

#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    const std::vector<sirkit::TreeDistanceSumEdge<std::int64_t>> edges{{0, 1, 2}, {1, 2, 4}};
    sirkit::TreeDistanceSums<std::int64_t> distances(3, edges);
    distances.add(1, 5);
    std::cout << distances.sum_at(0) << ' ' << distances.sum_path(0, 2) << '\n'; // 10 30
}
