// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Rollback_Lazy_Segment_Tree.cpp"

#include <array>
#include <cassert>
#include <cstddef>

long long Combine(long long a, long long b)
{
    return a + b;
}

long long Zero()
{
    return 0;
}

long long AddToSum(long long add, long long sum, std::size_t length)
{
    return sum + add * static_cast<long long>(length);
}

int main()
{
    const std::array<long long, 4> values{1, 2, 3, 4};
    sirkit::RollbackLazySegmentTree<long long, long long, Combine, Zero, AddToSum, Combine> tree(values);
    const auto before = tree.snapshot();
    tree.apply(1, 4, 5);
    assert(tree.all_prod() == 25);
    tree.rollback(before);
    assert(tree.prod(0, 3) == 6);
}
