// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{};
    verify_in >> n;
    std::vector<sirkit::DynamicTreeDistanceEdge<>> edges(n - 1);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v >> weight;
        --u;
        --v;
    }
    sirkit::DynamicTreeDistances<> tree(n, edges);
    int queries{};
    verify_in >> queries;
    while (queries-- > 0)
    {
        int type{}, first{};
        std::int64_t second{};
        verify_in >> type >> first >> second;
        if (type == 1)
        {
            tree.set_weight(first - 1, second);
        }
        else
        {
            verify_out << *tree.distance(first - 1, static_cast<int>(second) - 1) << '\n';
        }
    }
}
