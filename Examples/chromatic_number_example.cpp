// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Chromatic_Number.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    const std::array edges{std::pair{0, 1}, std::pair{1, 2}, std::pair{2, 0}};
    assert(sirkit::chromatic_number(3, edges) == 3);
    const auto result = sirkit::minimum_vertex_coloring(3, edges);
    assert(result && result->color_count == 3);
    for (const auto &[u, v] : edges)
    {
        assert(result->colors[u] != result->colors[v]);
    }
}
