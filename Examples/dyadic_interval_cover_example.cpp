// SPDX-License-Identifier: CC0-1.0
#include "../Algorithm/Dyadic_Interval_Cover.cpp"

#include <iostream>

int main()
{
    for (const auto &[left, right] : sirkit::dyadic_interval_cover(3, 9))
    {
        std::cout << left << ' ' << right << '\n';
    }
}
