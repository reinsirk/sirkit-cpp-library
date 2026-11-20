// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Bipartite_Matching.cpp"
#include <iostream>
#include <utility>

using namespace sirkit;
int main() {
    BipartiteMatching graph(3,4);
    graph.reserve_neighbors(0,2);
    graph.add_edge(0,2); graph.add_edge(0,1);
    graph.add_edge(1,2); graph.add_edge(2,0);
    std::cout << graph.solve() << '\n'; // 3
    for (auto [left,right] : graph.pairs()) std::cout << left << ':' << right << ' ';
    std::cout << '\n'; // 0:1 1:2 2:0
    std::cout << graph.match_left(0) << ' ' << graph.match_right(2) << '\n'; // 1 1
    std::cout << graph.left_matches().size() << ' ' << graph.right_matches().size() << '\n'; // 3 4
    std::cout << graph.min_vertex_cover().size() << ' ' << graph.max_independent_set().size() << '\n'; // 3 4
    graph.add_edge(1,3);
    std::cout << graph.is_maximum() << ' ' << graph.solve() << '\n'; // 0 3
    auto copy=graph; copy.clear_matching();
    std::cout << copy.matching_size() << ' ' << copy.solve() << ' ' << graph.matching_size() << '\n'; // 0 3 3
    copy.clear_edges();
    std::cout << copy.left_size() << ' ' << copy.right_size() << ' ' << copy.edge_count() << '\n'; // 3 4 0
    auto moved=std::move(copy);
    std::cout << copy.empty() << ' ' << moved.solve() << '\n'; // 1 0
    moved.reset(1,2); moved.add_edge(0,1);
    std::cout << moved.neighbors(0).front() << ' ' << moved.solve() << '\n'; // 1 1
    moved.clear(); std::cout << moved.empty() << '\n'; // 1
}
