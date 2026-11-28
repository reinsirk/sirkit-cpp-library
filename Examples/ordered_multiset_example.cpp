// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Ordered_Multiset.cpp"

#include <cassert>

int main()
{
    sirkit::OrderedMultiset<int> values;
    values.insert(2);
    values.insert(5, 2);
    assert(values.count_less(5) == 1);
    assert(*values.kth(2) == 5);
    assert(values.erase(5) == 1 && values.size() == 2);
}
