// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Backedge_Absorption.cpp"

#include <array>
#include <iostream>
#include <utility>

int main()
{
    const std::array parents{-1, 0, 1};
    const std::array back{std::pair{1, 0}};
    const sirkit::TreeBackedgeAbsorption walk(parents, back);
    const std::array costs{5.0, 2.0, 99.0};
    std::cout << walk.expected_steps<double>().value() << '\n';
    std::cout << walk.expected_cost<double>(costs).value() << '\n';
}
