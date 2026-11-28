// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Josephus.cpp"

#include <iostream>

int main()
{
    std::cout << sirkit::josephus_survivor(7, 3) << '\n';
    for (const auto person : sirkit::josephus_order(7, 3))
    {
        std::cout << person << '\n';
    }
}
