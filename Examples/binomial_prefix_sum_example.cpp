// SPDX-License-Identifier: CC0-1.0
#include "../Math/Combination/Binomial_Prefix_Sum.cpp"
#include <cstdint>
#include <iostream>
#include <utility>
#include <vector>

int main()
{
    const std::vector<std::pair<std::uint32_t, std::uint32_t>> queries{{4, 2}, {5, 4}};
    for (const auto result : sirkit::binomial_prefix_sums(queries))
    {
        std::cout << result << '\n'; // 11, 31
    }
}
