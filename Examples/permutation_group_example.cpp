// SPDX-License-Identifier: CC0-1.0
#include "../Math/Permutation_Group.cpp"
#include <array>
#include <cassert>
#include <vector>

int main()
{
    const std::vector<sirkit::PermutationGroup::Permutation> generators{{1, 2, 0}};
    const sirkit::PermutationGroup group(3, generators);
    assert(group.order_mod(998'244'353) == 3);
    assert(group.contains(std::array{2, 0, 1}));
    assert(!group.contains(std::array{1, 0, 2}));
}
