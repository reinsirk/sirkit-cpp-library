// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Sqrt_Tree.cpp"

#include <algorithm>
#include <iostream>
#include <limits>
#include <vector>

int main()
{
    const sirkit::SqrtTree tree(std::vector{8, 3, 6, 1, 5}, std::numeric_limits<int>::max(),
                                [](int a, int b)
                                {
                                    return std::min(a, b);
                                });
    std::cout << tree.prod(1, 4) << '\n'; // 1
}
