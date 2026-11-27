// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Prefix_Sum.cpp"
#include <array>
#include <cstdint>
#include <iostream>

int main()
{
    const std::array<std::int64_t, 4> values{3, -1, 4, 2};
    const sirkit::PrefixSum<std::int64_t> sums(values);
    std::cout << sums.prefix(2) << '\n'; // 2
    std::cout << sums.sum(1, 4) << '\n'; // 5
}
