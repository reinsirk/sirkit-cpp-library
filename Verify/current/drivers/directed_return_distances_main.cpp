// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::ReturnDistanceArc> edges(m);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v >> weight;
        --u;
        --v;
    }
    for (const auto distance : sirkit::directed_return_distances(n, edges))
    {
        if (distance)
        {
            verify_out << *distance << '\n';
        }
        else
        {
            verify_out << -1 << '\n';
        }
    }
}
