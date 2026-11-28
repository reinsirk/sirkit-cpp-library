// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Path_Affine_Sum.cpp"

#include <array>
#include <cassert>
#include <cstdint>

int main()
{
    using T = std::int64_t;
    using Tree = sirkit::TreePathAffineSum<T>;
    const std::array<T, 2> values{7, 10};
    const std::array edges{Tree::Edge{0, 1, {2, 6}, {3, 4}}};
    Tree tree(values, edges);
    assert(tree.all_prod() == 33);
    assert(tree.sum_from(1) == 35);
    tree.set_edge(0, {1, 0});
    tree.set_vertex(0, 8);
    assert(tree.sum_from(1) == 18);
}
