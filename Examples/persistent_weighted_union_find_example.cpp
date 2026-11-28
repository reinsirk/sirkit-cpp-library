// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Persistent_Weighted_Union_Find.cpp"

#include <cassert>

int main()
{
    sirkit::PersistentWeightedUnionFind<long long> uf(4);
    const auto empty = uf.version();
    uf.unite_checked(0, 1, 4);
    const auto first = uf.version();
    uf.unite_checked(1, 2, -2);
    assert(uf.try_diff(0, 2) == 2);
    assert(!uf.try_diff(0, 2, first));

    uf.unite_checked(0, 3, 10, empty);
    assert(uf.issame(0, 3));
    assert(!uf.issame(0, 1));
    assert(uf.try_diff(0, 1, first) == 4);
}
