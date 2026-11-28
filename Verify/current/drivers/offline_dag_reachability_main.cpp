// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int n{}, m{}, q{};
    verify_in >> n >> m >> q;
    std::vector<std::pair<int, int>> edges(m), queries(q);
    for (auto &list : {&edges, &queries})
    {
        for (auto &[u, v] : *list)
        {
            verify_in >> u >> v;
            --u;
            --v;
        }
    }
    for (const bool reachable : sirkit::offline_dag_reachability(n, edges, queries))
    {
        verify_out << (reachable ? "Yes\n" : "No\n");
    }
}
