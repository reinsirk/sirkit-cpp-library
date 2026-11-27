// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Persistent_Array.cpp"
#include <iostream>

int main()
{
    sirkit::PersistentArray<int> array(3, 0);
    auto *initial = array.root;
    auto *first = array.update(0, 5);
    auto *second = array.update(initial, 0, 8);
    std::cout << array.get(initial, 0) << '\n'; // 0
    std::cout << array.get(first, 0) << '\n';   // 5
    std::cout << array.get(second, 0) << '\n';  // 8
}
