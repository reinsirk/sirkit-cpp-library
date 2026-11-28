// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Difference_Constraints.cpp"

#include <array>
#include <iostream>

int main()
{
    using Constraint = sirkit::DifferenceConstraint;
    const std::array constraints{Constraint{0, 1, 4}, Constraint{1, 0, -2}};
    const auto answer = sirkit::solve_difference_constraints(2, constraints);
    const auto &[lower, upper] = answer.bounds[1];
    std::cout << lower.value() << ' ' << upper.value() << '\n'; // 2 4
}
