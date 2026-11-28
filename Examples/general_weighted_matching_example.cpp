// SPDX-License-Identifier: CC0-1.0
#include "../Graph/General_Weighted_Matching.cpp"

#include <array>
#include <iostream>

int main()
{
    const std::array edges{sirkit::GeneralWeightedMatchingEdge{0, 2, 1},
                           sirkit::GeneralWeightedMatchingEdge{1, 3, 1},
                           sirkit::GeneralWeightedMatchingEdge{1, 2, 3}};
    const auto answer = sirkit::maximum_weight_matching(4, edges);
    std::cout << answer.weight << '\n';
    for (const auto &[u, v] : answer.edges)
    {
        std::cout << u << ' ' << v << '\n';
    }
}
