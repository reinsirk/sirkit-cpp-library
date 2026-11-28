// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    int n{}, m{}, source{}, target{};
    verify_in >> n >> m >> source >> target;
    std::vector<sirkit::ShortestPathEdge> edges(m);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v >> weight;
    }
    const auto route = sirkit::lexicographic_shortest_path(n, edges, source, target, true);
    const auto &vertices = route.value().vertices;
    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        verify_out << vertices[i] << (i + 1 == vertices.size() ? '\n' : ' ');
    }
}
