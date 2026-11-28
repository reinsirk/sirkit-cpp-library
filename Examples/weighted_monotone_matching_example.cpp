// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Weighted_Monotone_Matching.cpp"
#include <array>
#include <cassert>
#include <cstddef>
#include <vector>

int main()
{
    const std::array edges{sirkit::WeightedMatchEdge{0, 2, 8},
                           sirkit::WeightedMatchEdge{1, 1, 6},
                           sirkit::WeightedMatchEdge{2, 2, 9}};
    const auto answer = sirkit::maximum_weight_monotone_matching(edges);
    assert(answer.weight == 15);
    assert((answer.indices == std::vector<std::size_t>{1, 2}));
}
