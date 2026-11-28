// SPDX-License-Identifier: CC0-1.0
#include "../Math/Floor_Sum_Moments.cpp"
#include <iostream>

int main()
{
    const auto values = sirkit::floor_sum_moments(4, 7, 2, 4);                                         // q={0,0,1,1}
    std::cout << values.sum << ' ' << values.index_weighted << ' ' << values.sum_of_squares() << '\n'; // 2 5 2
}
