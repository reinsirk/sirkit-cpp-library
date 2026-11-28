// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Undirected_Xor_Walks.cpp"

#include <array>
#include <cassert>

int main()
{
    const std::array edges{sirkit::XorWalkEdge{0, 1, 6}, sirkit::XorWalkEdge{1, 2, 7},
                           sirkit::XorWalkEdge{0, 2, 4}};
    const sirkit::UndirectedXorWalks walks(4, edges);
    assert(walks.min_xor(0, 2) == 1);
    assert(walks.max_xor(0, 2) == 4);
    assert(walks.attainable(0, 2, 4));
    assert(!walks.min_xor(0, 3));
    assert(walks.count_pairs_at_most(2) == 2);
}
