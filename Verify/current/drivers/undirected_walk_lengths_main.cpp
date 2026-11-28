// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{}, source{}, target{};
    std::uint64_t length{};
    verify_in >> n >> m >> length >> source >> target;
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const sirkit::UndirectedWalkLengths paths(n, edges, source - 1);
    const auto vertices = paths.vertices_on_walks(target - 1, length);
    if (vertices.empty())
    {
        verify_out << -1 << '\n';
        return 0;
    }
    verify_out << vertices.size() << '\n';
    for (const int v : vertices)
    {
        verify_out << v + 1 << '\n';
    }
}
