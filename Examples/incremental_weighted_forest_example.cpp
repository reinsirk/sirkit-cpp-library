// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Incremental_Weighted_Forest.cpp"

#include <iostream>

int main()
{
    sirkit::IncrementalWeightedForest forest(4);
    forest.link(0, 1, 3);
    forest.link(1, 2, 4);
    std::cout << forest.distance(0, 2).value() << '\n';
    const auto [first, second, distance] = forest.diameter(1);
    std::cout << first << ' ' << second << ' ' << distance << '\n';
}
