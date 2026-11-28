// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<sirkit::TreePermutationEdge> edges(n - 1);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
        weight = 1;
    }
    const sirkit::TreePermutationDistance assignment(n, edges);
    verify_out << assignment.optimal_count() << '\n';
}
