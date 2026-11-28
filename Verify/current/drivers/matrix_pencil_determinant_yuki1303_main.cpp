// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;
using Matrix = std::vector<std::vector<std::int64_t>>;
using Adjacency = std::vector<std::vector<bool>>;

void AddEdge(Matrix &matrix, std::size_t u, std::size_t v)
{
    if (u < matrix.size())
    {
        ++matrix[u][u];
    }
    if (v < matrix.size())
    {
        ++matrix[v][v];
        --matrix[u][v];
        --matrix[v][u];
    }
}

std::uint32_t CountTrees(const Adjacency &graph, const std::vector<std::size_t> &vertices, bool allow_special_edge)
{
    const auto dimension = vertices.size() - 1;
    Matrix roads(dimension, std::vector<std::int64_t>(dimension));
    Matrix additions(dimension, std::vector<std::int64_t>(dimension));
    for (std::size_t i = 0; i < vertices.size(); ++i)
    {
        for (std::size_t j = i + 1; j < vertices.size(); ++j)
        {
            if (graph[vertices[i]][vertices[j]])
            {
                AddEdge(roads, i, j);
            }
            else if (allow_special_edge)
            {
                AddEdge(additions, i, j);
            }
        }
    }
    const auto counts = sirkit::matrix_pencil_determinant(roads, additions);
    return allow_special_edge ? (counts[0] + counts[1]) % kMod : counts[0];
}
} // namespace

int main()
{
    std::size_t n{}, m{};
    verify_in >> n >> m;
    Adjacency graph(n, std::vector<bool>(n));
    for (std::size_t i = 0; i < m; ++i)
    {
        std::size_t u{}, v{};
        verify_in >> u >> v;
        --u;
        --v;
        graph[u][v] = graph[v][u] = true;
    }

    std::vector<bool> visited(n);
    std::vector<std::vector<std::size_t>> components;
    for (std::size_t u = 0; u < n; ++u)
    {
        if (visited[u])
        {
            continue;
        }
        auto &component = components.emplace_back(1, u);
        visited[u] = true;
        for (std::size_t i = 0; i < component.size(); ++i)
        {
            for (std::size_t v = 0; v < n; ++v)
            {
                if (!visited[v] && graph[component[i]][v])
                {
                    visited[v] = true;
                    component.push_back(v);
                }
            }
        }
    }
    if (components.size() == 1)
    {
        verify_out << 0 << '\n'
                   << CountTrees(graph, components.front(), true) << '\n';
        return 0;
    }

    auto inconvenient = n * n;
    std::uint64_t ways = 1;
    std::size_t best_product = 0, bridge_count = 0;
    for (std::size_t i = 0; i < components.size(); ++i)
    {
        const auto count = components[i].size();
        inconvenient -= count * count;
        ways = ways * CountTrees(graph, components[i], false) % kMod;
        for (std::size_t j = 0; j < i; ++j)
        {
            const auto product = components[j].size() * count;
            if (product > best_product)
            {
                best_product = product;
                bridge_count = product;
            }
            else if (product == best_product)
            {
                bridge_count += product;
            }
        }
    }
    verify_out << inconvenient - 2 * best_product << '\n'
               << ways * bridge_count % kMod << '\n';
}
