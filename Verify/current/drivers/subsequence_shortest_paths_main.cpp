// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{}, q{};
    verify_in >> n >> m >> q;
    std::vector<sirkit::SubsequencePathEdge<std::int64_t>> edges(m);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v >> weight;
        --u;
        --v;
    }
    std::vector<int> schedule(q);
    for (auto &id : schedule)
    {
        verify_in >> id;
        --id;
    }
    const sirkit::SubsequenceShortestPaths<std::int64_t> paths(n, edges, schedule, 0);
    verify_out << paths.distance_to(n - 1).value_or(-1) << '\n';
}
