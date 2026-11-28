// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Subset_Medians.cpp"

#include <cstddef>
#include <iostream>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<int, int>> edges{{0, 1}, {1, 2}};
    const sirkit::TreeSubsetMedians medians(3, edges);
    const auto choose = [](std::size_t n, std::size_t k)
    {
        std::size_t result = 1;
        for (std::size_t i = 0; i < k; ++i)
        {
            result = result * (n - i) / (i + 1);
        }
        return result;
    };
    std::cout << medians.median_count_sum(2, choose) << '\n'; // 2 + 3 + 2 = 7
}
