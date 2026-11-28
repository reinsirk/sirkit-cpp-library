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
    sirkit::Graph first(n);
    for (int i = 1; i < n; ++i)
    {
        int u, v;
        verify_in >> u >> v;
        --u;
        --v;
        first[u].push_back(v);
        first[v].push_back(u);
    }
    std::vector<std::pair<int, int>> edges(n - 1);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    const sirkit::VirtualTree second(n, edges);
    const sirkit::CentroidDecomposition decomposition(first);
    std::vector<std::vector<std::pair<int, int>>> all(n), exclude(n);
    for (int v = 0; v < n; ++v)
    {
        const auto &path = decomposition.ancestors(v);
        for (std::size_t i = 0; i < path.size(); ++i)
        {
            all[path[i].vertex].emplace_back(v, path[i].distance);
            if (i > 0)
            {
                exclude[path[i].vertex].emplace_back(v, path[i - 1].distance);
            }
        }
    }

    std::vector<int> terminals, counts, weight(n);
    std::vector<std::uint64_t> sums;
    const auto contribution = [&](const std::vector<std::pair<int, int>> &group) -> std::uint64_t {
        if (group.size() < 2)
        {
            return 0;
        }
        terminals.clear();
        std::uint64_t total_sum = 0;
        const int total_count = static_cast<int>(group.size());
        for (const auto &[v, w] : group)
        {
            terminals.push_back(v);
            weight[v] = w;
            total_sum += static_cast<std::uint64_t>(w);
        }
        const auto nodes = second.build(terminals);
        counts.assign(nodes.size(), 0);
        sums.assign(nodes.size(), 0);
        std::uint64_t result = 0;
        for (std::size_t i = nodes.size() - 1; i > 0; --i)
        {
            const auto &[v, parent, distance, terminal] = nodes[i];
            if (terminal)
            {
                ++counts[i];
                sums[i] += static_cast<std::uint64_t>(weight[v]);
            }
            const auto near = static_cast<std::uint64_t>(counts[i]);
            const auto far = static_cast<std::uint64_t>(total_count - counts[i]);
            result += static_cast<std::uint64_t>(distance) * (sums[i] * far + (total_sum - sums[i]) * near);
            counts[parent] += counts[i];
            sums[parent] += sums[i];
        }
        return result;
    };
    std::uint64_t answer = 0;
    for (int v = 0; v < n; ++v)
    {
        answer += contribution(all[v]) - contribution(exclude[v]);
    }
    verify_out << answer * 2 << '\n';
}
