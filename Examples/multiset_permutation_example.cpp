// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Multiset_Permutation.cpp"

#include <iostream>
#include <vector>

int main()
{
    const sirkit::MultisetPermutation order(std::vector{3, 1, 3});
    const auto second = order.unrank(1).value();
    std::cout << order.count().to_string() << '\n'; // 3
    for (const int value : second)
    {
        std::cout << value << ' '; // 3 1 3
    }
    std::cout << '\n';
}
