// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Common_Interval_Decomposition.cpp"
#include <array>
#include <cassert>

int main()
{
    const std::array permutation{0, 2, 1, 3};
    const auto tree = sirkit::common_interval_decomposition(permutation);
    assert(tree.size() == 6);
    assert(tree.front().kind == sirkit::CommonIntervalKind::increasing);
    assert(tree[2].left == 1 && tree[2].right == 3);
    assert(tree[2].kind == sirkit::CommonIntervalKind::decreasing);
}
