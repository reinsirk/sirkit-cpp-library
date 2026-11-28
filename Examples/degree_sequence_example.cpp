// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Degree_Sequence.cpp"

#include <array>
#include <iostream>

int main()
{
    constexpr std::array degrees{1, 1, 2};
    const auto edges = sirkit::realize_degree_sequence(degrees).value();
    for (const auto &[u, v] : edges)
    {
        std::cout << u << ' ' << v << '\n';
    }
}
