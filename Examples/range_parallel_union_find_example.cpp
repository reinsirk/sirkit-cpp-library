// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Range_Parallel_Union_Find.cpp"

#include <cassert>
#include <vector>

int main()
{
    sirkit::RangeParallelUnionFind uf(5);
    std::vector<int> sums{2, 3, 5, 7, 11};
    const auto count = uf.merge_range(0, 2, 3, [&](int keep, int absorbed)
                                      {
                                          sums[keep] += sums[absorbed];
                                      });
    assert(count == 3);
    assert(uf.issame(0, 4));
    assert(sums[uf.root(0)] == 18);
}
