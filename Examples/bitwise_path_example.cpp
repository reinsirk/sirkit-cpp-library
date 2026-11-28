// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Bitwise_Path.cpp"

#include <iostream>
#include <vector>

int main()
{
    const std::vector<sirkit::BitwisePathEdge> edges{{0, 1, 1}, {1, 2, 2}, {0, 2, 4}};
    const auto minimum = sirkit::minimum_or_path(3, edges, 0, 2);
    const auto maximum = sirkit::maximum_and_path(3, edges, 0, 2);
    std::cout << minimum->value << ' ' << maximum->value << '\n'; // 3 4
}
