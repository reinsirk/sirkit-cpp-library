// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstddef>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::JohnsonEdge> edges(m);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v >> weight;
    }
    auto print_row = [](int, auto row)
    {
        for (std::size_t v = 0; v < row.size(); ++v)
        {
            if (row[v])
            {
                verify_out << *row[v];
            }
            else
            {
                verify_out << "INF";
            }
            verify_out << (v + 1 == row.size() ? '\n' : ' ');
        }
    };
    if (!sirkit::for_each_johnson_distance_row(n, edges, print_row))
    {
        verify_out << "NEGATIVE CYCLE\n";
    }
}
