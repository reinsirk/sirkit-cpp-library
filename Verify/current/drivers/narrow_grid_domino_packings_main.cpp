// SPDX-License-Identifier: CC0-1.0
#include <cstdint>
#include "Verify/current/common/fast_io.hpp"

int main()
{
    std::uint64_t width = 0;
    verify_in >> width;
    verify_out << narrow_grid_domino_packings(3, width, DominoPackingMode::kMaximal) << '\n';
}
