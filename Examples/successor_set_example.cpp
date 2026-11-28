// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Successor_Set.cpp"
#include <cassert>

int main()
{
    sirkit::SuccessorSet slots(4);
    assert(slots.erase(2));
    assert(slots.take(2) == 3);
    assert(slots.take_cyclic(2) == 0);
    assert(slots.remaining() == 1);
}
