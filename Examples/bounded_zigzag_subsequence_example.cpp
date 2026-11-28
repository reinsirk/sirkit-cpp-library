// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Bounded_Zigzag_Subsequence.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <vector>

int main()
{
    const std::array<std::int64_t, 3> heights{1, 3, 2};
    const std::array<std::int64_t, 3> weights{5, 4, 6};
    const auto answer = sirkit::bounded_zigzag_subsequence(heights, weights, 1, 3);
    assert(answer.weight == 15);
    const std::vector<std::size_t> positions{0, 1, 2};
    assert(answer.indices == positions);
}
