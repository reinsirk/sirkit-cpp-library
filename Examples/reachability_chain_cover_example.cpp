// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Reachability_Chain_Cover.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 2}, std::pair{1, 2}, std::pair{2, 3}, std::pair{2, 4}};
    const auto chains = sirkit::minimum_reachability_chain_cover(5, edges);
    assert(chains.size() == 2);
}
