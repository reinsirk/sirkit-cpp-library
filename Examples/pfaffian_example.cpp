// SPDX-License-Identifier: CC0-1.0
#include "../Math/Pfaffian.cpp"
#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    constexpr std::uint32_t kMod = 998244353;
    const std::vector<std::vector<std::uint32_t>> matrix{{0, 3}, {kMod - 3, 0}};
    assert(sirkit::pfaffian_mod(matrix) == 3);
}
