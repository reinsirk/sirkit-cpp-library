// SPDX-License-Identifier: CC0-1.0
#include "../Math/Floor_Path_Product.cpp"
#include <iostream>
#include <string>

int main()
{
    const auto concatenate = [](const std::string &a, const std::string &b)
    {
        return a + b;
    };
    std::cout << sirkit::floor_path_product(3, 5, 2, 4, std::string("X"), std::string("Y"), std::string(), concatenate) << '\n'; // XYXXY
}
