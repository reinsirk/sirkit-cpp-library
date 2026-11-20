// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Offline_Dynamic_Connectivity.cpp"
#include <cstdint>
#include <iostream>
#include <vector>

using namespace sirkit;
int main() {
    std::vector<std::int64_t> weights{3, 5, 7, 11};
    OfflineDynamicConnectivity graph(weights);
    const auto edge = graph.add_edge(0, 1);
    graph.query_same(0, 2);
    graph.query_size(0);
    graph.query_component_count();
    graph.add_vertex(1, 4);
    graph.query_component_sum(0);
    const auto parallel = graph.add_edge(1, 0);
    graph.remove_edge(edge);
    graph.query_same(0, 1);
    graph.remove_edge(parallel);
    graph.query_component_sum(1);
    for (const auto answer : graph.solve()) std::cout << answer << ' ';
    std::cout << '\n';
    auto copy = graph;
    copy.add_edge(1, 2);
    copy.query_component_sum(2);
    std::cout << copy.solve().back() << ' ' << graph.query_count() << '\n';
    graph.clear_history();
    graph.query_component_sum(1);
    std::cout << graph.solve()[0] << '\n';
    auto moved = std::move(graph);
    std::cout << moved.size() << ' ' << graph.empty() << '\n';
    graph.reset(2);
    graph.reserve_edges(10); graph.reserve_additions(10); graph.reserve_queries(10);
    graph.add_edge(0, 1); graph.query_component_count();
    std::cout << graph.solve()[0] << '\n';
    graph.clear();
}
