// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Persistent_Li_Chao_Tree.cpp"
#include <iostream>

int main()
{
    sirkit::PersistentLiChaoTree tree({0, 1, 3});
    const auto a = tree.add_line({}, {2, 5});
    const auto b = tree.add_segment(a, 0, 3, {0, 2});
    std::cout << *tree.get(a, 1) << '\n'; // 7
    std::cout << *tree.get(b, 1) << '\n'; // 2
    std::cout << *tree.get(b, 3) << '\n'; // 11
}
