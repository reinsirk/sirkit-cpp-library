// SPDX-License-Identifier: CC0-1.0
#include "../Math/Ferrers_Rook.cpp"

#include <iostream>
#include <vector>

int main()
{
    const std::vector<std::uint32_t> heights{1, 2};
    const auto rook = sirkit::ferrers_rook_counts(heights);
    const auto hits = sirkit::ferrers_hit_counts(heights);
    std::cout << rook[1] << ' ' << hits[2] << '\n'; // 3 1
}
