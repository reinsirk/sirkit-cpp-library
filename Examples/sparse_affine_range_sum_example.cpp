// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Sparse_Affine_Range_Sum.cpp"

#include <cassert>

int main()
{
    sirkit::SparseAffineRangeSum<> data(1'000'000'000);
    data.range_affine(10, 20, 3, 7);
    const auto count = data.stored_nodes();
    assert(data.range_sum(0, 15) == 35);
    assert(data.all_sum() == 70);
    assert(data.stored_nodes() == count);
}
