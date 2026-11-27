// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Minimum_Spanning_Tree.cpp"
#include <cassert>
#include <cstddef>
#include <vector>

int main()
{
    const std::vector<sirkit::SpanningTreeEdge> edges{{0, 1, 4}, {1, 2, 1}, {0, 2, 2}};
    const auto ids = sirkit::minimum_spanning_forest(3, edges);
    assert((ids == std::vector<std::size_t>{1, 2}));
}
