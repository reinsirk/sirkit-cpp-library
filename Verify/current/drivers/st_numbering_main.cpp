// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int tests;
    verify_in >> tests;
    std::vector<std::pair<int, int>> edges;
    while (tests-- != 0)
    {
        int n, m, source, sink;
        verify_in >> n >> m >> source >> sink;
        edges.resize(m);
        for (auto &[u, v] : edges)
        {
            verify_in >> u >> v;
        }
        const auto ranks = st_numbering(n, source, sink, edges);
        if (!ranks)
        {
            verify_out << "No\n";
            continue;
        }
        verify_out << "Yes\n";
        for (int v = 0; v < n; ++v)
        {
            if (v != 0)
            {
                verify_out << ' ';
            }
            verify_out << (*ranks)[v];
        }
        verify_out << '\n';
    }
}
