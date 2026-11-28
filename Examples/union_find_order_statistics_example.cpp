// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Union_Find_Order_Statistics.cpp"
#include <iostream>

int main()
{
    sirkit::UnionFindOrderStatistics tree(std::vector{30, 10, 30, 20});
    tree.merge(0, 1);
    tree.merge(2, 1);
    std::cout << *tree.kth(0, 0) << '\n';        // 10
    std::cout << tree.count(0, 30) << '\n';      // 2
    std::cout << tree.count_less(2, 20) << '\n'; // 1
}
