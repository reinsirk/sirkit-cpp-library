// SPDX-License-Identifier: CC0-1.0
#include "../Math/Polynomial_Matrix_Product.cpp"

#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    const std::vector<std::vector<std::vector<std::int64_t>>> matrix{
        {{1}, {0, 1}},
        {{}, {1}}};
    const auto product = sirkit::polynomial_matrix_interval_product(matrix, 2, 3);
    std::cout << product[0][1] << '\n'; // 9
}
