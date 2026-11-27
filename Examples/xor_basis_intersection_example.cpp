// SPDX-License-Identifier: CC0-1.0
#include "../Math/Xor_Basis_Intersection.cpp"
#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    const std::array<std::uint64_t, 2> left{3, 5}, right{6, 8};
    const auto common = sirkit::xor_basis_intersection(left, right);
    assert(common.size() == 1 && common[0] == 6);
}
