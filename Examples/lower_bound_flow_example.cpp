// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Lower_Bound_Flow.cpp"

#include <cassert>
#include <vector>

int main()
{
    sirkit::LowerBoundFlow<int> graph(3);
    graph.add_edge(0, 1, 2, 4);
    graph.add_edge(1, 2, 1, 3);
    const auto minimum = graph.min_flow(0, 2).value();
    const auto maximum = graph.max_flow(0, 2).value();
    assert(minimum.value == 2 && minimum.edge_flow == std::vector<int>({2, 2}));
    assert(maximum.value == 3 && maximum.edge_flow == std::vector<int>({3, 3}));
}
