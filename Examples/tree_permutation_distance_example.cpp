// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Permutation_Distance.cpp"

#include <iostream>
#include <vector>

int main()
{
    const std::vector<sirkit::TreePermutationEdge> edges{{0, 1, 2}, {1, 2, 3}};
    const sirkit::TreePermutationDistance assignment(3, edges);
    std::cout << *assignment.maximum() << ' ' << assignment.optimal_count() << '\n'; // 10 3
}
