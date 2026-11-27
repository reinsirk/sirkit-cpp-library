// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Hierarchical_Bitset.cpp"

#include <cassert>

int main()
{
    sirkit::HierarchicalBitset set("010010");
    assert(set.next(2) == 4);
    assert(set.prev(2) == 1);
    set.insert(3);
    set.erase(4);
    assert(set.next(2) == 3);
    assert(!set.next(4));
    assert(set.count() == 2);
}
