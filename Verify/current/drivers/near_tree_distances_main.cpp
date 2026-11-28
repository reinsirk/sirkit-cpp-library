// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n{}, m{}, q{};
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const sirkit::NearTreeDistances distances(n, edges);
    verify_in >> q;
    while (q-- > 0)
    {
        int u{}, v{};
        verify_in >> u >> v;
        verify_out << distances.distance(u - 1, v - 1) << '\n';
    }
}
