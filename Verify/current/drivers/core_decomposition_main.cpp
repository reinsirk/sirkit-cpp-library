// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <utility>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<std::int64_t> weights(n);
    for (auto &weight : weights)
    {
        verify_in >> weight;
    }
    std::vector<std::pair<int, int>> edges(m);
    for (auto &[u, v] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
    }
    verify_out << sirkit::weighted_core_decomposition<std::int64_t>(n, edges, weights).degeneracy << '\n';
}
