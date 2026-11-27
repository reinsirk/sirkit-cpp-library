// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_B_Matching.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array cap{1, 1, 1};
    const std::array edges{sirkit::TreeBMatchingEdge{0, 1, 4},
                           sirkit::TreeBMatchingEdge{1, 2, 7}};
    const auto best = sirkit::maximum_weight_tree_b_matching(cap, edges);
    assert(best.weight == 7 && best.edge_ids.size() == 1 && best.edge_ids[0] == 1);
}
