// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Interval_Cover_Subsets.cpp"

#include <array>
#include <iostream>

int main()
{
    using Choice = sirkit::IntervalCoverChoice<int>;
    const std::array choices{Choice{0, 1}, Choice{1, 2}, Choice{0, 2}};
    std::cout << sirkit::count_interval_cover_subsets<int>(0, 2, choices) << '\n';
}
