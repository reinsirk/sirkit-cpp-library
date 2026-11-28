// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Bipartite_Matching_Analysis.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    using Role = sirkit::BipartiteEdgeRole;
    const std::array edges{std::pair{0, 0}, std::pair{0, 1}, std::pair{1, 1}};
    const auto result = sirkit::analyze_bipartite_matching(2, 2, edges);
    assert(result.matching_size == 2);
    assert(result.edge_role[0] == Role::mandatory && result.edge_role[1] == Role::forbidden);
    assert(!result.left_can_be_free[0]);
}
