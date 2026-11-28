// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Segment_Tree_2D.cpp"

#include <cassert>

int main()
{
    sirkit::SegmentTree2D table(3, 4, 0, [](int lhs, int rhs) noexcept
                                {
                                    return lhs + rhs;
                                });
    table.set(1, 2, 5);
    table.set(2, 3, 7);
    assert(table.prod(0, 0, 2, 3) == 5);
    assert(table.all_prod() == 12);
}
