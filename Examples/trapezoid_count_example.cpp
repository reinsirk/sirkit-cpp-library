// SPDX-License-Identifier: CC0-1.0
#include "../Geometry/Trapezoid_Count.cpp"

#include <array>
#include <iostream>

int main()
{
    const std::array<sirkit::TrapezoidPoint, 5> points{{{0, 2}, {0, 5}, {1, 0}, {2, 1}, {2, 4}}};
    const auto [trapezoids, parallelograms] = sirkit::count_trapezoids(points);
    std::cout << trapezoids << '\n'
              << parallelograms << '\n';
}
