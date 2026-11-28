// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Chinese_Postman_Route.cpp"

#include <array>
#include <iostream>

int main()
{
    const std::array edges{sirkit::PostmanEdge{0, 1, 2}, sirkit::PostmanEdge{1, 2, 3}};
    std::cout << sirkit::minimum_postman_route(3, edges, 0)->cost << '\n';
    const auto open = sirkit::minimum_postman_route(3, edges, 0, 2).value();
    std::cout << open.cost << '\n';
    for (const auto vertex : open.vertices)
    {
        std::cout << vertex << ' ';
    }
    std::cout << '\n';
}
