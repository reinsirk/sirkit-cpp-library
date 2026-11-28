// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Vertex_Deletion_Components.cpp"

#include <array>
#include <iostream>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}};
    const auto report = sirkit::vertex_deletion_components(4, edges);
    std::cout << report.original_count << '\n';     // 2
    std::cout << report.after_deleting[1] << '\n';  // 3
    std::cout << report.separated_pairs[1] << '\n'; // 1
}
