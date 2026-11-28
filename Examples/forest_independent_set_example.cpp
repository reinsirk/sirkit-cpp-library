// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Forest_Independent_Set.cpp"

#include <array>
#include <iostream>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
    const sirkit::ForestIndependentSet forest(3, edges);
    std::cout << forest.count<int>() << '\n'; // 5
    const auto middle = forest.count_by_vertex<int>()[1];
    std::cout << middle.excluded << ' ' << middle.included << '\n'; // 4 1
    const std::array weights{4, 6, 5};
    const auto optimum = forest.maximum_weight<int>(weights);
    std::cout << optimum.weight << '\n'; // 9
}
