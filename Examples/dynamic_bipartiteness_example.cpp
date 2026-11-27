// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Dynamic_Bipartiteness.cpp"

#include <cassert>
#include <vector>

int main()
{
    sirkit::DynamicBipartiteness graph(3);
    graph.add_edge(0, 1);
    graph.add_edge(1, 2);
    graph.query_bipartite();
    const auto diagonal = graph.add_edge(0, 2);
    graph.query_bipartite();
    graph.remove_edge(diagonal);
    graph.query_bipartite();
    const auto answers = graph.solve();
    assert((answers == std::vector<bool>{true, false, true}));
}
