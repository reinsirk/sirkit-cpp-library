// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Sparse_Segment_Tree.cpp"

#include <cassert>
#include <cstdint>

namespace
{
constexpr std::int64_t Add(std::int64_t a, std::int64_t b) noexcept
{
    return a + b;
}

constexpr std::int64_t Zero() noexcept
{
    return 0;
}
}

int main()
{
    sirkit::SparseSegmentTree<std::int64_t, Add, Zero> data(1'000'000'000'000);
    data.set(3, 7);
    data.set(1'000'000'000'000 - 1, 2);
    assert(data.prod(0, 100) == 7);
    assert(data.all_prod() == 9);
    data.set(3, 0);
    assert(data.stored_points() == 1);
    assert(data.get(3) == 0);
}
