#include "../Graph/Incremental_Bridges.cpp"

#include <cassert>

int main()
{
    sirkit::IncrementalBridges graph(3);
    const auto first = graph.add_edge(0, 1);
    graph.add_edge(1, 2);
    assert(graph.is_bridge(first));
    assert(graph.bridge_count() == 2);

    graph.add_edge(2, 0);
    assert(!graph.is_bridge(first));
    assert(graph.bridge_count() == 0);
    assert(graph.same_two_edge_component(0, 2));
}
