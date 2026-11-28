// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int left{}, right{}, m{};
    verify_in >> left >> right >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const auto analysis = sirkit::analyze_bipartite_matching(left, right, edges);
    for (const auto role : analysis.edge_role)
    {
        verify_out << (role == sirkit::BipartiteEdgeRole::forbidden ? "No\n" : "Yes\n");
    }
}
