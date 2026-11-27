// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Persistent_Affine_Range_Sum.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main()
{
    const std::vector<std::int64_t> values{1, 2, 3};
    sirkit::PersistentAffineRangeSum<> tree(values);
    const auto initial = tree.initial_root();
    const auto changed = tree.range_affine(initial, 0, 3, 2, 1);
    const auto mixed = tree.range_copy(changed, initial, 1, 2);
    assert(tree.all_sum(mixed) == 12);
    assert(tree.all_sum(changed) == 15);
    assert(tree.all_sum(initial) == 6);
}
