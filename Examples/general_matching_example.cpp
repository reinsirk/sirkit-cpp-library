// SPDX-License-Identifier: CC0-1.0
#include "../Graph/General_Matching.cpp"
#include <iostream>
#include <utility>

using namespace sirkit;
int main() {
    GeneralMatching graph(6);
    graph.reserve_edges(8);
    graph.add_edge(0,1); graph.add_edge(1,2); graph.add_edge(2,0);
    graph.add_edge(3,4); graph.add_edge(4,5); graph.add_edge(5,3);
    graph.add_edge(0,0); // 自己loopを無視する。
    std::cout << graph.solve() << ' ' << graph.is_perfect() << '\n';
    graph.add_edge(2,5);
    std::cout << graph.matching_size() << ' ' << graph.is_maximum() << '\n';
    std::cout << graph.solve() << ' ' << graph.is_perfect() << '\n';
    for (const auto& [u,v] : graph.pairs()) std::cout << u << '-' << v << ' ';
    std::cout << '\n';
    for (auto mate : graph.partners()) std::cout << mate << ' ';
    std::cout << '\n';
    std::cout << graph.mate(2) << ' ' << graph.edges().size() << ' ' << graph.edge_count() << '\n';
    auto copy=graph; copy.clear_matching();
    std::cout << graph.matching_size() << ' ' << copy.matching_size() << ' ' << copy.solve() << '\n';
    GeneralMatching moved(std::move(copy));
    std::cout << copy.empty() << ' ' << moved.size() << '\n';
    moved.clear_edges(); moved.reset(2); moved.add_edge(0,1);
    std::cout << moved.solve() << '\n';
    swap(moved,copy); copy.clear();
    std::cout << moved.empty() << ' ' << copy.empty() << ' ' << copy.is_perfect() << '\n';
}
