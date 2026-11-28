// SPDX-License-Identifier: CC0-1.0
#include "../DataStructure/Sparse_Table_2D.cpp"

#include <algorithm>
#include <cassert>

int main()
{
    const std::vector<std::vector<int>> grid{{4, 2, 8}, {6, 5, 1}};
    const sirkit::SparseTable2D table(grid, [](int lhs, int rhs)
                                      {
                                          return std::min(lhs, rhs);
                                      });
    assert(table.prod(0, 0, 2, 2) == 2);
    assert(table.prod(0, 1, 2, 3) == 1);
}
