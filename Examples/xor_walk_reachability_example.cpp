// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Xor_Walk_Reachability.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array edges{sirkit::XorWalkEdge{0, 1, 7}, sirkit::XorWalkEdge{1, 1, 6}};
    const sirkit::XorWalkReachability walks(3, edges, 0);
    assert(walks.min_xor(1) == 1);
    assert(walks.max_xor(1) == 7);
    assert(walks.reachable(1, 7));
    assert(!walks.min_xor(2));
}
