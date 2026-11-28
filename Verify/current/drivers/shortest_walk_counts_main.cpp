// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    struct Ways
    {
        unsigned value;
        void operator+=(const Ways &other)
        {
            constexpr unsigned kModulus = 1000000007;
            value += other.value;
            if (value >= kModulus)
            {
                value -= kModulus;
            }
        }
    };

    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::ShortestPathEdge> edges(m);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v;
        --u;
        --v;
        weight = 1;
    }
    verify_out << sirkit::count_shortest_walks<Ways>(n, edges, 0, true).count[n - 1].value << '\n';
}
