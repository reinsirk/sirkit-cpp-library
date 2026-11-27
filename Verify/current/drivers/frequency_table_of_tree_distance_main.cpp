// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

int main()
{
    int n;
    verify_in >> n;
    Graph graph(n);
    for (int i = 1; i < n; ++i)
    {
        int a, b;
        verify_in >> a >> b;
        graph[a].push_back(b);
        graph[b].push_back(a);
    }

    const auto frequency = tree_distance_frequency(graph);
    for (int d = 1; d < n; ++d)
    {
        if (d > 1)
        {
            verify_out << ' ';
        }
        verify_out << frequency[d];
    }
    verify_out << '\n';
}
