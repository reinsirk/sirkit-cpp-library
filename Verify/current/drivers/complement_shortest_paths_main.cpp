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
    const auto answer = sirkit::complement_bfs(n, edges, 0, 998244353);
    if (answer.distance.back() == -1)
    {
        verify_out << -1 << '\n';
    }
    else
    {
        verify_out << answer.paths.back() << '\n';
    }
}
