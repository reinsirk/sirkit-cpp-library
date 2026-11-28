// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <cstdint>
#include <vector>

int main()
{
    int n{}, m{};
    verify_in >> n >> m;
    std::vector<sirkit::JohnsonEdge> edges(m);
    for (auto &[u, v, weight] : edges)
    {
        verify_in >> u >> v >> weight;
        --u;
        --v;
    }
    auto print_sum = [](int, auto row)
    {
        std::int64_t sum = 0;
        for (const auto &distance : row)
        {
            sum += distance.value_or(0);
        }
        verify_out << sum << '\n';
    };
    (void)sirkit::for_each_johnson_distance_row(n, edges, print_sum);
}
