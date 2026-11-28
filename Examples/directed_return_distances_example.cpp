// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Directed_Return_Distances.cpp"

#include <iostream>
#include <vector>

int main()
{
    const std::vector<sirkit::ReturnDistanceArc> edges{{0, 1, 3}, {1, 0, 4}, {2, 1, 1}};
    const auto values = sirkit::directed_return_distances(3, edges);
    for (auto value : values)
    {
        if (value)
        {
            std::cout << *value << '\n'; // 7, 7
        }
        else
        {
            std::cout << "-1\n"; // vertex 2
        }
    }
}
