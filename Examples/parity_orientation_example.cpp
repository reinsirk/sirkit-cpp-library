// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Parity_Orientation.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
    const auto even = sirkit::orient_with_outdegree_parity(3, edges);
    assert((even && (*even)[0] == std::pair{1, 0} && (*even)[1] == std::pair{1, 2}));

    const std::array<std::uint8_t, 3> parity{1, 0, 1};
    const auto odd_ends = sirkit::orient_with_outdegree_parity(3, edges, parity);
    assert((odd_ends && (*odd_ends)[0] == std::pair{0, 1} && (*odd_ends)[1] == std::pair{2, 1}));
}
