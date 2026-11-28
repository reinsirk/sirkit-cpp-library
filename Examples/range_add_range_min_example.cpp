// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Range_Add_Range_Min.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array<long long, 4> values{3, 1, 4, 2};
    sirkit::RangeAddRangeMin<long long> tree(values);
    tree.add(1, 3, 5);
    assert(tree.minimum(0, 3) == 3);
    assert(tree.all_minimum() == 2);
    assert(!tree.minimum(2, 2));
}
