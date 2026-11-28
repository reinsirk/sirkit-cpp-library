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
    const auto chains = sirkit::minimum_reachability_chain_cover(n, edges);
    std::vector<int> color(n);
    for (std::size_t index = 0; index < chains.size(); ++index)
    {
        for (const int vertex : chains[index])
        {
            color[vertex] = static_cast<int>(index + 1);
        }
    }
    for (int i = 0; i < n; ++i)
    {
        verify_out << color[i] << (i + 1 == n ? '\n' : ' ');
    }
}
