// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <utility>
#include <vector>

int main()
{
    int tests{};
    verify_in >> tests;
    for (int test = 0; test < tests; ++test)
    {
        int n{};
        verify_in >> n;
        std::vector<std::pair<int, int>> edges(n);
        for (auto &edge : edges)
        {
            verify_in >> edge.first;
            --edge.first;
        }
        for (auto &edge : edges)
        {
            verify_in >> edge.second;
            --edge.second;
        }
        const auto trails = sirkit::minimum_eulerian_trail_decomposition<true>(n, edges);
        const auto count = static_cast<int>(trails.size());
        verify_out << n - (count == 1 && trails.front().circuit() ? 0 : count) << '\n';
    }
}
