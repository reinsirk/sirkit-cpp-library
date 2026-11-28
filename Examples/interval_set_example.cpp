// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Interval_Set.cpp"

#include <cassert>

int main()
{
    sirkit::IntervalSet<int> used;
    used.add(0, 3);
    used.add(3, 5);
    assert(used.mex() == 5);
    used.erase(2, 4);
    assert(used.mex() == 2);
    assert(used.size() == 3);
}
