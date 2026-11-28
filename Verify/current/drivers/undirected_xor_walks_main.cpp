// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int test_cases{};
    verify_in >> test_cases;
    while (test_cases-- > 0)
    {
        int n{}, m{};
        std::uint64_t bound{};
        verify_in >> n >> m >> bound;
        std::vector<sirkit::XorWalkEdge> edges(m);
        for (auto &[u, v, weight] : edges)
        {
            verify_in >> u >> v >> weight;
            --u;
            --v;
        }
        const sirkit::UndirectedXorWalks walks(n, edges);
        verify_out << walks.count_pairs_at_most(bound) << '\n';
    }
}
