// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Undirected_Pseudoforest.cpp"

#include <array>
#include <cassert>
#include <utility>

int main()
{
    using Edge = std::pair<int, int>;
    const std::array edges{Edge{0, 1}, Edge{1, 2}, Edge{2, 0}, Edge{1, 3}};
    const auto graph = sirkit::UndirectedPseudoforest::build(4, edges).value();
    assert(graph.cycle(graph.cycle_id(3)).vertices.size() == 3);
    assert(graph.root(3) == 1 && graph.depth(3) == 1);
    assert(graph.unique_path(1, 3));
    assert(!graph.unique_path(0, 3));
    assert(graph.distance(0, 3) == 2);
}
