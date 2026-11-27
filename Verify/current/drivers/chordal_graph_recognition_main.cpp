// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <utility>
#include <vector>

int main()
{
    int n, m;
    verify_in >> n >> m;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
    }

    const auto result = recognize_chordal_graph(n, edges);
    const auto &vertices = result.is_chordal ? result.perfect_elimination_order : result.induced_cycle;
    if (result.is_chordal)
    {
        verify_out << "YES\n";
    }
    else
    {
        verify_out << "NO\n"
                   << vertices.size() << '\n';
    }
    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        if (i != 0)
        {
            verify_out << ' ';
        }
        verify_out << vertices[i];
    }
    verify_out << '\n';
}
