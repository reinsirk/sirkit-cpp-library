// SPDX-License-Identifier: CC0-1.0
#include "../Math/Stirling_Columns.cpp"
#include <iostream>

int main()
{
    const auto first = sirkit::stirling_first_column(4, 2);
    const auto second = sirkit::stirling_second_column(4, 2);
    std::cout << first.back() << '\n';  // 11
    std::cout << second.back() << '\n'; // 7
}
