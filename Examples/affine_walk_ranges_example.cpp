// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Affine_Walk_Ranges.cpp"

#include <array>
#include <iostream>

int main()
{
    using Edge = sirkit::AffineWalkEdge;
    const std::array edges{Edge{0, 1, false, 2}, Edge{0, 1, false, 5}, Edge{1, 2, true, 0}};
    const auto ranges = sirkit::affine_walk_ranges(3, edges, 0);
    std::cout << ranges[2].minimum << ' ' << ranges[2].maximum << '\n';
}
