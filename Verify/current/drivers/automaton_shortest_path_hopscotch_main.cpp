// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{}, m{}, source{}, target{};
    verify_in >> n >> m;
    std::vector<sirkit::AutomatonPathEdge> edges(m);
    for (auto &[u, v, label, weight] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
        label = 0;
        weight = 1;
    }
    verify_in >> source >> target;
    auto next = [](int state, int) { return (state + 1) % 3; };
    auto accept = [](int state) { return state == 0; };
    const auto path = sirkit::automaton_shortest_path(n, edges, source - 1, target - 1, 3, 0, next, accept);
    if (path)
    {
        verify_out << path->distance / 3 << '\n';
    }
    else
    {
        verify_out << -1 << '\n';
    }
}
