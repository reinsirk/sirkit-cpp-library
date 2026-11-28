// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Short_Simple_Path.cpp"

#include <iostream>
#include <vector>

int main()
{
    const std::vector<std::vector<int>> graph{{1, 2}, {3}, {3}, {}};
    const std::vector<int> starts{0}, ends{3};
    if (const auto path = sirkit::find_short_simple_path(graph, 3, starts, ends))
    {
        for (const int vertex : *path)
        {
            std::cout << vertex << '\n'; // 0, 1, 3 または 0, 2, 3
        }
    }
}
