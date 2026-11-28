// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Hub_Sparse_Distances.cpp"

#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    const std::vector<std::uint64_t> gates{10, 100, 10};
    const std::vector<sirkit::HubSparseDistances::Shortcut> shortcuts{{0, 1, 2}};
    const sirkit::HubSparseDistances graph(gates, shortcuts);
    std::cout << graph.distance(0, 2) << ' ' << graph.distance(1, 2) << '\n'; // 20 22
}
