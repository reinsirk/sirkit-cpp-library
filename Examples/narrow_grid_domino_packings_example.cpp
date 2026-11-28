// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Narrow_Grid_Domino_Packings.cpp"

#include <cassert>

int main()
{
    assert(sirkit::narrow_grid_domino_packings(3, 2, sirkit::DominoPackingMode::kPerfect) == 3);
    assert(sirkit::narrow_grid_domino_packings(3, 2, sirkit::DominoPackingMode::kMaximal) == 5);
}
