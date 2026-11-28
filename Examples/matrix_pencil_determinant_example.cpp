// SPDX-License-Identifier: CC0-1.0
#include "../Math/Matrix_Pencil_Determinant.cpp"

#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    const std::vector<std::vector<std::int64_t>> constant{{1, 2}, {3, 4}};
    const std::vector<std::vector<std::int64_t>> linear{{1, 0}, {0, 0}};
    const auto coefficients = sirkit::matrix_pencil_determinant<7>(constant, linear);
    std::cout << coefficients[0] << ' ' << coefficients[1] << ' ' << coefficients[2] << '\n'; // 5 4 0
}
