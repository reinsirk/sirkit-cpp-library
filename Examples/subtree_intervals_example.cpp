// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Subtree_Intervals.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array parents{-1, 0, 0, 1, 1};
    const sirkit::SubtreeIntervals index(parents);
    assert(index.range(1) == (std::pair{1, 4}));
    assert(index.vertex_at(2) == 3);
    assert(index.contains(1, 4));
}
