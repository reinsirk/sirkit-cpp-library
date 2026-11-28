// SPDX-License-Identifier: CC0-1.0
#include "../Math/Xor_Two_Independent_Sets.cpp"

#include <initializer_list>
#include <iostream>

int main()
{
    sirkit::XorTwoIndependentSets bases;
    for (auto value : {1u, 1u, 2u, 3u, 3u})
    {
        bases.insert(value);
    }
    std::cout << bases.rank() << '\n'; // 4
    for (const auto &group : bases.groups())
    {
        for (auto id : group)
        {
            std::cout << id << ' ';
        }
        std::cout << '\n';
    }
}
