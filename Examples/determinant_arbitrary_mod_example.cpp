// SPDX-License-Identifier: CC0-1.0
#include "../Math/Determinant_Arbitrary_Mod.cpp"
#include <cassert>

int main()
{
    const std::vector<std::vector<std::int64_t>> a{{2, 1}, {3, 4}};
    assert(sirkit::determinant_mod(a, 6) == 5);
}
