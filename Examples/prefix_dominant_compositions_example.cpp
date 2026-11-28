// SPDX-License-Identifier: CC0-1.0
#include "../Math/Prefix_Dominant_Compositions.cpp"

#include <iostream>

int main()
{
    const auto count = sirkit::prefix_dominant_composition_counts(3, 1, 1);
    std::cout << count[1] << ' ' << count[2] << ' ' << count[3] << '\n'; // 1 3 6
}
