// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Range_Add_State_Ids.cpp"
#include <cassert>

int main()
{
    sirkit::RangeAddStateIds states(100);
    const auto first = states.add(10, 20, 3);
    assert(states.add(10, 20, -3) == 0);
    assert(states.add(10, 20, 3) == first);
}
