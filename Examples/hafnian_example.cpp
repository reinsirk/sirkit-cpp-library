// SPDX-License-Identifier: CC0-1.0
#include "../Math/Hafnian.cpp"
#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    const std::vector<std::vector<std::int64_t>> matrix{
        {0, 1, 2, 3},
        {1, 0, 4, 5},
        {2, 4, 0, 6},
        {3, 5, 6, 0}};
    std::cout << sirkit::hafnian(matrix) << '\n'; // 1*6 + 2*5 + 3*4 = 28
}
