// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n{}, q{};
    verify_in >> n;
    std::vector<std::pair<int, int>> edges(n);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const auto component = sirkit::UndirectedPseudoforest::build(n, edges).value();
    verify_in >> q;
    while (q-- > 0)
    {
        int u{}, v{};
        verify_in >> u >> v;
        verify_out << (component.unique_path(u - 1, v - 1) ? "Yes\n" : "No\n");
    }
}
