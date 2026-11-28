// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <vector>

int main()
{
    int tests{};
    verify_in >> tests;
    while (tests-- > 0)
    {
        int n{}, m{};
        verify_in >> n >> m;
        std::vector<sirkit::WalkCongruenceEdge> edges(m);
        for (auto &[u, v, weight] : edges)
        {
            verify_in >> u >> v;
            --u;
            --v;
            weight = 1;
        }
        auto period = sirkit::WalkWeightCongruence(n, edges).period(0);
        if (period != 0)
        {
            for (const int factor : {2, 5})
            {
                while (period % factor == 0)
                {
                    period /= factor;
                }
            }
        }
        verify_out << (period == 1 ? "Yes" : "No") << '\n';
    }
}
