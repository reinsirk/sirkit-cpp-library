// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Bipartite_Coloring.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    using Edge = std::pair<int, int>;
    const std::array path{Edge{0, 1}, Edge{1, 2}};
    const auto coloring = sirkit::bipartite_coloring(3, path);
    assert(coloring.is_bipartite());
    assert(coloring.color[0] == coloring.color[2]);
    const std::array triangle{Edge{0, 1}, Edge{1, 2}, Edge{2, 0}};
    const auto witness = sirkit::bipartite_coloring(3, triangle);
    assert(witness.odd_cycle && witness.odd_cycle->vertices.size() == 3);
}
