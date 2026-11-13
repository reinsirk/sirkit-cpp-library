// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_SCC_TEST_SOURCE
#define SIRKIT_SCC_TEST_SOURCE "../../../Graph/SCC.cpp"
#endif
#include SIRKIT_SCC_TEST_SOURCE
#include SIRKIT_SCC_TEST_SOURCE
#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <type_traits>
#include <utility>
#include <vector>

namespace {
using Groups = std::vector<std::vector<int>>;
static_assert(std::is_default_constructible_v<scc_graph>);
static_assert(std::is_constructible_v<scc_graph, int>);
static_assert(!std::is_convertible_v<int, scc_graph>);
static_assert(std::is_same_v<decltype(&scc_graph::add_edge), void (scc_graph::*)(int, int)>);
static_assert(std::is_same_v<decltype(&scc_graph::scc), Groups (scc_graph::*)()>);
static_assert(std::is_same_v<decltype(&scc_graph::build), int (scc_graph::*)()>);
template<class G> concept HasConstScc = requires(const G& g) { g.scc(); };
template<class G> concept HasComponents = requires(G& g) { g.components(); };
static_assert(!HasConstScc<scc_graph>);
static_assert(!HasComponents<scc_graph>);
std::uint64_t checks = 0;
void check(bool condition) { ++checks; if (!condition) std::abort(); }
void default_argument(scc_graph graph = {}) { check(graph.scc().empty()); }

void verify(scc_graph& graph, const std::vector<std::pair<int, int>>& edges) {
    const int n = graph.size();
    const auto groups = graph.scc();
    check(graph.edge_count() == edges.size());
    check(graph.component_count() == static_cast<int>(groups.size()));
    std::vector<int> id(n, -1);
    for (int c = 0; c < static_cast<int>(groups.size()); ++c) {
        check(!groups[c].empty());
        check(std::is_sorted(groups[c].begin(), groups[c].end()));
        for (int v : groups[c]) {
            check(0 <= v && v < n);
            check(id[v] == -1);
            id[v] = c;
        }
    }
    auto reach = std::vector(n, std::vector<bool>(n));
    for (int v = 0; v < n; ++v) reach[v][v] = true;
    for (auto [u, v] : edges) reach[u][v] = true;
    for (int k = 0; k < n; ++k)
        for (int u = 0; u < n; ++u)
            for (int v = 0; v < n; ++v)
                reach[u][v] = reach[u][v] || (reach[u][k] && reach[k][v]);
    for (int u = 0; u < n; ++u) {
        check(id[u] >= 0 && graph.id(u) == id[u] && graph.ids()[u] == id[u]);
        for (int v = 0; v < n; ++v) {
            check((id[u] == id[v]) == (reach[u][v] && reach[v][u]));
            if (reach[u][v] && id[u] != id[v]) check(id[u] < id[v]);
        }
    }
    // 結果のコピーを書き換えても内部の成分IDは変わらない。
    auto returned_copy = groups;
    if (!returned_copy.empty()) returned_copy.front().clear();
    check(graph.ids() == id);
    check(graph.scc() == groups);
    check(graph.build() == static_cast<int>(groups.size()));
    check(graph.ids() == id);
}
}

int main() {
    scc_graph empty = {};
    default_argument();
    verify(empty, {});
    {
        scc_graph graph(6);
        std::vector<std::pair<int, int>> edges;
        // 無関係な成分の順は固定せず、到達可能性で検査する。
        for (auto [u, v] : std::vector<std::pair<int, int>>{{0,0},{0,1},{0,1},{1,0},{2,3},{4,5}}) {
            graph.add_edge(u, v); edges.emplace_back(u, v);
        }
        verify(graph, edges);
        for (auto [u, v] : std::vector<std::pair<int, int>>{{1,2},{3,4},{5,0}}) {
            graph.add_edge(u, v); edges.emplace_back(u, v);
            verify(graph, edges);
        }
        auto copied = graph;
        graph.reset();
        verify(graph, {});
        verify(copied, edges);
        auto moved = std::move(copied);
        verify(copied, {});
        verify(moved, edges);
        copied = moved;
        verify(copied, edges);
        copied = std::move(moved);
        verify(moved, {});
        verify(copied, edges);
    }
    std::mt19937_64 random(20261103);
    for (int n = 0; n <= 28; ++n) {
        for (int trial = 0; trial < 40; ++trial) {
            scc_graph graph(n);
            std::vector<std::pair<int, int>> edges;
            for (int stage = 0; stage < 4; ++stage) {
                if (n) for (int i = 0; i < n / 2 + 1; ++i) {
                    const int u = static_cast<int>(random() % n);
                    const int v = static_cast<int>(random() % n);
                    graph.add_edge(u, v); edges.emplace_back(u, v);
                }
                verify(graph, edges);
            }
        }
    }
    std::printf("SCC ACL API checks=%" PRIu64 "\n", checks);
}
