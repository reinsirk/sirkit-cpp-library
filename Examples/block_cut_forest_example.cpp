// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Block_Cut_Forest.cpp"

#include <array>
#include <iostream>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 0}, std::pair{2, 3}};
    const auto forest = sirkit::block_cut_forest(4, edges);
    for (std::size_t block = 4; block < forest.size(); ++block)
    {
        for (const int vertex : forest[block])
        {
            std::cout << vertex << ' ';
        }
        std::cout << '\n';
    }
}
