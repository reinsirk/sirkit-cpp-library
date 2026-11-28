// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Path_Matching.cpp"

#include <array>
#include <iostream>

int main()
{
    const std::array weights{3, 5, 3};
    const sirkit::PathMatching<int> matching(weights);
    std::cout << matching.best(1) << ' ' << matching.best(2) << '\n'; // 5 6
    for (const auto index : matching.edge_indices(2))
    {
        std::cout << index << ' '; // 0 2
    }
    std::cout << '\n';
}
