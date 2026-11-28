// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::BitwisePathEdge> edges(m);
    for (auto &[u, v, label] : edges)
    {
        verify_in >> u >> v >> label;
        --u;
        --v;
    }
    verify_out << sirkit::minimum_or_path(n, edges, 0, n - 1, false)->value << '\n';
}
