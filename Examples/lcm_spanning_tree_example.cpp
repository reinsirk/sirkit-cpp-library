// SPDX-License-Identifier: CC0-1.0
#include "../Graph/LCM_Spanning_Tree.cpp"

#include <array>
#include <cstdint>
#include <iostream>

int main()
{
    const std::array<std::uint32_t, 4> labels{6, 9, 12, 18};
    std::uint64_t weight_sum = 0;
    for (const auto &edge : sirkit::minimum_lcm_spanning_tree(labels))
    {
        weight_sum += edge.weight;
    }
    std::cout << weight_sum << '\n'; // 48
}
