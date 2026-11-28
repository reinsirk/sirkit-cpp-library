// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Heap_Orderings.cpp"

#include <array>
#include <iostream>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{0, 2}};
    for (const auto count : sirkit::tree_heap_orderings(3, edges))
    {
        std::cout << count << '\n'; // 2, 1, 1
    }
}
