// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Tree_Mo.cpp"

#include <array>
#include <cstddef>
#include <iostream>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
    const std::array queries{std::pair{0, 2}, std::pair{2, 2}};
    sirkit::TreeMo tree(3, edges);
    int active = 0;
    std::array<int, 2> answer{};
    tree.run(queries, [&](int) { ++active; }, [&](int) { --active; },
             [&](std::size_t i) { answer[i] = active; });
    for (const int count : answer)
    {
        std::cout << count << '\n';
    }
}
