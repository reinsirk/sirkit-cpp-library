// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{}, m{}, group_count{}, source_count{};
    verify_in >> n >> m >> group_count >> source_count;
    std::vector<int> groups(n);
    for (int &group : groups)
    {
        verify_in >> group;
    }
    std::vector<sirkit::GroupedShortestPathSource> sources(source_count);
    for (auto &[vertex, group] : sources)
    {
        verify_in >> vertex;
        group = groups[--vertex];
    }
    std::vector<sirkit::ShortestPathEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to >> weight;
        --from;
        --to;
    }
    const auto closest = sirkit::two_nearest_source_groups(n, edges, sources, true);
    for (int v = 0; v < n; ++v)
    {
        const auto &[first, second] = closest[v];
        const auto &answer = first && first->group == groups[v] ? second : first;
        if (answer)
        {
            verify_out << answer->distance;
        }
        else
        {
            verify_out << -1;
        }
        verify_out << (v + 1 == n ? '\n' : ' ');
    }
}
