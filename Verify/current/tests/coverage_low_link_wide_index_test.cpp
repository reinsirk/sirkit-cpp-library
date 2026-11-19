// SPDX-License-Identifier: CC0-1.0
#define SIRKIT_LOW_LINK_COMPACT_INDEX_LIMIT 2
#include "../../../Graph/Low_Link.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <stdexcept>
#include <vector>

int main() {
    using sirkit::detail::LowLinkArcCount;
    assert(LowLinkArcCount(4, 9) == 8);
    bool rejected = false;
    try { (void)LowLinkArcCount(5, 9); }
    catch (const std::length_error&) { rejected = true; }
    assert(rejected);

    // 連結パスと孤立点。幅切替後も橋と各二重連結成分は1辺ずつ。
    sirkit::UndirectedLowLink graph(5);
    graph.add_edge(0, 1);
    graph.add_edge(1, 2);
    graph.add_edge(2, 3);
    assert(graph.build() == 2);
    assert((graph.bridges() == std::vector<std::size_t>{0, 1, 2}));
    assert((graph.articulation_points() == std::vector{1, 2}));
    assert(graph.build(true) == 2);
    auto edges = graph.biconnected_edge_components();
    std::ranges::sort(edges);
    assert((edges == std::vector<std::vector<std::size_t>>{{0}, {1}, {2}}));
    auto vertices = graph.biconnected_vertex_components();
    for (auto& block : vertices) std::ranges::sort(block);
    std::ranges::sort(vertices);
    assert((vertices == std::vector<std::vector<int>>{{0, 1}, {1, 2}, {2, 3}, {4}}));
}
