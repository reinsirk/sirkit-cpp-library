// SPDX-License-Identifier: CC0-1.0
#include "../Math/Tetration_Mod.cpp"
#include <cassert>

int main()
{
    assert(sirkit::tetration_mod(2, 4, 1000) == 536);
}
