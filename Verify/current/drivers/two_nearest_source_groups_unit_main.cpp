// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <string>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::ShortestPathEdge> edges(m);
    for (auto &[from, to, weight] : edges)
    {
        verify_in >> from >> to;
        --from;
        --to;
        weight = 1;
    }
    std::string status;
    verify_in >> status;
    std::vector<sirkit::GroupedShortestPathSource> sources;
    for (int v = 0; v < n; ++v)
    {
        if (status[v] == 'S')
        {
            sources.push_back({v, v});
        }
    }
    const auto closest = sirkit::two_nearest_source_groups(n, edges, sources, true);
    for (int v = 0; v < n; ++v)
    {
        if (status[v] == 'D')
        {
            verify_out << closest[v][0]->distance + closest[v][1]->distance << '\n';
        }
    }
}
