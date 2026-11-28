// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{}, source{}, target{};
    std::uint64_t steps{};
    verify_in >> n >> m >> steps >> source >> target;
    --source;
    --target;
    std::vector<std::pair<int, int>> edges(m);
    bool adjacent = false;
    bool endpoint_has_other_neighbor = false;
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
        if ((u == source && v == target) || (u == target && v == source))
        {
            adjacent = true;
        }
        else if (u == source || v == source || u == target || v == target)
        {
            endpoint_has_other_neighbor = true;
        }
    }
    if ((steps & 1U) != 0 || endpoint_has_other_neighbor)
    {
        verify_out << "Yes\n";
        return 0;
    }
    if (!adjacent || steps < 6)
    {
        verify_out << "No\n";
        return 0;
    }
    // The edge between the endpoints contributes one move; the new bridge is used twice.
    const auto cycle = sirkit::shortest_odd_cycle(n, edges);
    verify_out << (!cycle.empty() && cycle.size() <= steps - 3 ? "Yes\n" : "No\n");
}
