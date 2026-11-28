// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const auto graph = sirkit::UndirectedPseudoforest::build(n, edges);
    verify_out << (graph && graph->cycle_count() == graph->component_count() ? "Yes\n" : "No\n");
}
