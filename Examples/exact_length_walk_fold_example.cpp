// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Exact_Length_Walk_Fold.cpp"

#include <functional>
#include <iostream>
#include <vector>

int main()
{
    const std::vector<sirkit::ExactLengthWalkArc<unsigned>> edges{{0, 1, 1}, {0, 2, 1}, {1, 3, 1}, {2, 3, 1}};
    const auto counts = sirkit::fold_exact_length_walks(4, edges, 0, 2, 0U, 1U, std::plus<>{}, std::multiplies<>{});
    std::cout << counts[3] << '\n'; // 2
}
