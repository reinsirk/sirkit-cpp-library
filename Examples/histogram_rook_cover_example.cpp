// SPDX-License-Identifier: CC0-1.0
#include "../Math/Histogram_Rook_Cover.cpp"

#include <array>
#include <cstdint>
#include <iostream>

int main()
{
    constexpr std::array<std::uint64_t, 3> heights{2, 1, 2};
    std::cout << sirkit::count_histogram_rook_covers(heights) << '\n'; // 17
}
