// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Range_Edge_Graph.cpp"

#include <cassert>
#include <cstddef>

int main()
{
    sirkit::RangeEdgeGraph graph(8);
    graph.add_range_edge(0, 4, 4, 8, 3);
    graph.add_edge(7, 0, 2);
    const auto adjacent = graph.graph();
    assert(adjacent.size() == static_cast<std::size_t>(graph.vertex_count()));
    assert(adjacent[7].back().to == 0 && adjacent[7].back().w == 2);
}
