// SPDX-License-Identifier: CC0-1.0
#include "../Math/Modular_Kth_Root.cpp"

#include <cassert>

int main()
{
    const auto found = sirkit::modular_kth_root(3, 8, 13);
    assert(found && (*found * *found * *found) % 13 == 8);
    assert(!sirkit::modular_kth_root(2, 2, 13));
}
