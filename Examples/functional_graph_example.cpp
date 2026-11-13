// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Functional_Graph.cpp"
#include <cstdint>
#include <iostream>
#include <limits>
#include <vector>
int main() {
    const std::vector<int> next{4, 2, 1, 6, 3, 7, 5, 6, 8};
    FunctionalGraph graph(next);
    const auto info = graph.info(0);
    std::cout << info.entry << ' ' << info.depth << ' ' << info.length << '\n';
    for (const int v : graph.cycle(info.id)) std::cout << v << ' ';
    std::cout << '\n';
    std::cout << graph.kth_successor(0, 2) << ' '
              << graph.kth_successor(0, std::numeric_limits<std::uint64_t>::max()) << '\n';
    std::cout << graph.distance(0, 7) << ' ' << graph.distance(7, 0) << ' '
              << graph.reachable(0, 1) << '\n';
    std::cout << graph.component_size(0) << ' ' << graph.cycle_count() << ' '
              << graph.on_cycle(0) << '\n';
    auto copy = graph;
    copy.build(copy.successors());
    auto moved = std::move(copy);
    std::cout << copy.empty() << ' ' << moved.size() << '\n';
    copy.build(std::vector<int>{0});
    std::cout << copy.successor(0) << ' ' << graph.successors()[0] << '\n';
    moved.clear();
}
