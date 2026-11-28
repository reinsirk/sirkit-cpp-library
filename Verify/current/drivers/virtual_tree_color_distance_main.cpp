// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n;
    verify_in >> n;
    std::vector<std::pair<int, int>> edges(n - 1);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const sirkit::VirtualTree tree(n, edges);
    std::vector<std::vector<int>> groups(n);
    for (int v = 0; v < n; ++v)
    {
        int color;
        verify_in >> color;
        groups[color - 1].push_back(v);
    }

    std::int64_t answer = 0;
    std::vector<int> counts;
    for (const auto &group : groups)
    {
        if (group.size() < 2)
        {
            continue;
        }
        const auto nodes = tree.build(group);
        const int total = static_cast<int>(group.size());
        counts.assign(nodes.size(), 0);
        for (std::size_t i = nodes.size() - 1; i > 0; --i)
        {
            const auto &[vertex, parent, distance, terminal] = nodes[i];
            counts[i] += terminal;
            answer += std::int64_t{distance} * counts[i] * (total - counts[i]);
            counts[parent] += counts[i];
        }
    }
    verify_out << answer << '\n';
}
