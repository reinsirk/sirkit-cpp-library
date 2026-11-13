// SPDX-License-Identifier: CC0-1.0
#include "../Graph/Rerooting_DP.cpp"
#include <cstdint>
#include <iostream>
#include <utility>
struct State { std::int64_t sum=0,count=0; };
int main() {
    RerootingDP<State> tree(5);
    tree.reserve_edges(3); tree.reserve_neighbors(1,3);
    tree.add_edge(0,1); tree.add_edge(1,2); tree.add_edge(1,3);
    auto merge=[](const State& a,const State& b){ return State{a.sum+b.sum,a.count+b.count}; };
    auto vertex=[](const State& a,int){ return State{a.sum,a.count+1}; };
    auto edge=[](const State& a,int,int,std::size_t){ return State{a.sum+a.count,a.count}; };
    tree.build(State{},merge,vertex,edge);
    for (const auto& result : tree.values()) std::cout << result.sum << ' ';
    std::cout << '\n'; // 5 3 5 5 0
    std::cout << tree.component_count() << ' ' << tree[4].count << '\n'; // 2 1
    std::cout << tree.message(0).sum << ' ' << tree.message(0,true).sum << '\n'; // 1 5
    std::cout << tree.messages().size() << ' ' << tree.edge_count() << '\n'; // 6 3
    for (auto arc : tree.neighbors(1)) std::cout << arc << ' ';
    std::cout << '\n'; // 1 2 4
    auto [u,v]=tree.edge(0); std::cout << u << ' ' << v << '\n'; // 0 1
    auto copy=tree; copy.clear();
    std::cout << tree.size() << ' ' << copy.empty() << '\n'; // 5 1
    auto moved=std::move(tree);
    std::cout << tree.empty() << ' ' << moved.has_results() << '\n'; // 1 1
    tree.reset(1); tree.build(State{},merge,vertex,edge);
    std::cout << tree[0].sum << ' ' << tree[0].count << '\n'; // 0 1
}
