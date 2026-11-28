// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"
#include <cstddef>
#include <vector>

int main()
{
    std::size_t n = 0;
    verify_in >> n;
    std::vector<std::size_t> left(n), right_position(n + 1);
    for (auto &value : left)
    {
        verify_in >> value;
    }
    for (std::size_t i = 0; i < n; ++i)
    {
        std::size_t value = 0;
        verify_in >> value;
        right_position[value] = i;
    }
    std::vector<sirkit::WeightedMatchEdge> edges;
    for (std::size_t i = 0; i < n; ++i)
    {
        for (auto multiple = left[i]; multiple <= n; multiple += left[i])
        {
            edges.push_back({i, right_position[multiple], 1});
        }
    }
    verify_out << sirkit::maximum_weight_monotone_matching(edges).weight << '\n';
}
