// SPDX-License-Identifier: CC0-1.0
#include "../String/Cyclic_Xor_Matches.cpp"

#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    const std::array<std::uint64_t, 3> source{0, 2, 1}, target{1, 2, 3};
    const auto matches = sirkit::cyclic_xor_matches(source, target);
    assert(matches.size() == 1 && matches[0].shift == 1 && matches[0].mask == 3);
}
