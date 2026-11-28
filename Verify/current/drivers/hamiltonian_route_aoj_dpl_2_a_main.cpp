// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <optional>
#include <vector>

int main()
{
    std::size_t vertex_count{}, edge_count{};
    verify_in >> vertex_count >> edge_count;
    std::vector<std::optional<int>> edges(vertex_count * vertex_count);
    for (std::size_t edge = 0; edge < edge_count; ++edge)
    {
        std::size_t u{}, v{};
        int weight{};
        verify_in >> u >> v >> weight;
        edges[u * vertex_count + v] = weight;
    }
    const auto cycle = sirkit::shortest_hamiltonian_cycle(vertex_count, 0, [&](std::size_t u, std::size_t v)
                                                          {
                                                              return edges[u * vertex_count + v];
                                                          });
    verify_out << (cycle ? cycle->cost : -1) << '\n';
}
