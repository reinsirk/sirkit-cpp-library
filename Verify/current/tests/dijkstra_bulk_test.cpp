// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#include <cstdint>
#include <vector>
#ifdef SIRKIT_DIJKSTRA_TEST_PREDEFINED_EDGE
#define SIRKIT_GRAPH_TYPES_DEFINED
namespace sirkit {
struct Edge {
    int to;
    std::int64_t w;
    Edge() = delete;
    Edge(int destination, std::int64_t weight) : to(destination), w(weight) {}
};
using WGraph = std::vector<std::vector<Edge>>;
}
#endif
#ifndef SIRKIT_DIJKSTRA_TEST_SOURCE
#define SIRKIT_DIJKSTRA_TEST_SOURCE "../../../Graph/Dijkstra.cpp"
#endif
#include SIRKIT_DIJKSTRA_TEST_SOURCE
#include SIRKIT_DIJKSTRA_TEST_SOURCE
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <queue>
#include <random>
#include <span>
#include <stdexcept>
#include <tuple>

namespace {
using Input = std::vector<std::tuple<int, int, std::int64_t>>;
std::uint64_t checks = 0, cases = 0;
void require(bool value) {
    ++checks;
    if (!value) {
        std::fprintf(stderr, "Dijkstra regression failed: case=%ju check=%ju\n",
                     static_cast<std::uintmax_t>(cases), static_cast<std::uintmax_t>(checks));
        std::abort();
    }
}
std::vector<std::int64_t> bellman_reference(int n, const Input& edges, int source) {
    std::vector<std::int64_t> distance(n, kInf);
    distance[source] = 0;
    for (int pass = 1; pass < n; ++pass) {
        bool changed = false;
        for (auto [u, v, weight] : edges) {
            if (distance[u] != kInf && distance[u] + weight < distance[v]) {
                distance[v] = distance[u] + weight;
                changed = true;
            }
        }
        if (!changed) break;
    }
    return distance;
}
std::vector<std::int64_t> heap_reference(int n, const Input& edges, int source) {
    std::vector<std::vector<std::pair<int, std::int64_t>>> graph(n);
    for (auto [u, v, weight] : edges) graph[u].emplace_back(v, weight);
    using State = std::pair<std::int64_t, int>;
    std::priority_queue<State, std::vector<State>, std::greater<State>> pending;
    std::vector<std::int64_t> distance(n, kInf);
    distance[source] = 0;
    pending.emplace(0, source);
    while (!pending.empty()) {
        auto [value, u] = pending.top();
        pending.pop();
        if (value != distance[u]) continue;
        for (auto [v, weight] : graph[u]) {
            if (value + weight < distance[v]) {
                distance[v] = value + weight;
                pending.emplace(distance[v], v);
            }
        }
    }
    return distance;
}
void check_path(const WGraph& graph, const std::vector<int>& path, int source, int target,
                const std::vector<std::int64_t>& distance) {
    if (distance[target] == kInf) {
        require(path.empty());
        return;
    }
    require(!path.empty() && path.front() == source && path.back() == target);
    require(path.size() <= graph.size());
    std::int64_t total = 0;
    for (std::size_t i = 1; i < path.size(); ++i) {
        require(0 <= path[i] && path[i] < static_cast<int>(graph.size()));
        std::int64_t weight = kInf;
        for (const auto& edge : graph[path[i - 1]])
            if (edge.to == path[i]) weight = std::min(weight, edge.w);
        require(weight != kInf);
        total += weight;
    }
    require(total == distance[target]);
}
void inspect(int n, const Input& edges, int source, int target, bool use_bellman) {
    ++cases;
    const auto original = edges;
    WGraph graph(n);
    for (auto [u, v, weight] : edges) graph[u].emplace_back(v, weight);
    const auto expected = use_bellman ? bellman_reference(n, edges, source)
                                      : heap_reference(n, edges, source);
    std::vector<std::int64_t> distance{-17};
    std::vector<int> path{-17};
    Dijkstra(graph, distance, source);
    require(distance == expected);
    Dijkstra(graph, distance, source, target, path);
    require(distance == expected);
    check_path(graph, path, source, target, distance);
    Dijkstra(n, edges, distance, source);
    require(distance == expected);
    Dijkstra(n, edges, distance, source, target, path);
    require(distance == expected);
    check_path(graph, path, source, target, distance);
    const std::span<const Input::value_type> view(edges);
    Dijkstra(n, view, distance, source, target, path);
    require(distance == expected);
    check_path(graph, path, source, target, distance);
    // target==sourceでも、遠い頂点を含む全距離を求める。
    Dijkstra(n, edges, distance, source, source, path);
    require(distance == expected);
    require(path == std::vector<int>{source});
    require(edges == original);
}
template<class Error, class Function> void rejects(Function action) {
    bool caught = false;
    try { action(); } catch (const Error&) { caught = true; }
    require(caught);
}
void input_contracts() {
    std::vector<std::int64_t> distance{9};
    std::vector<int> path{8};
    rejects<std::length_error>([&] { Dijkstra(-1, Input{}, distance, 0); });
    rejects<std::length_error>([&] { Dijkstra(-1, Input{}, distance, 0, 0, path); });
    require(distance == std::vector<std::int64_t>{9} && path == std::vector<int>{8});
#ifndef SKIP_RUNTIME_CHECKS
    // 辺列版は未到達の行も構築時に検査し、出力を変更する前に拒否する。
    for (const Input& broken : {Input{{1, 7, 0}}, Input{{-1, 0, 0}}, Input{{0, -1, 0}},
                              Input{{3, 0, 0}}, Input{{0, 3, 0}}, Input{{1, 2, -1}},
                              Input{{1, 2, 1'000'000'001}}}) {
        rejects<std::out_of_range>([&] { Dijkstra(3, broken, distance, 0); });
        rejects<std::out_of_range>([&] { Dijkstra(3, broken, distance, 0, 0, path); });
        require(distance == std::vector<std::int64_t>{9} && path == std::vector<int>{8});
    }
    rejects<std::out_of_range>([&] { Dijkstra(0, Input{}, distance, 0); });
    rejects<std::out_of_range>([&] { Dijkstra(WGraph{}, distance, 0); });
    for (int invalid : {-1, 2}) {
        const WGraph graph(2);
        rejects<std::out_of_range>([&] { Dijkstra(2, Input{}, distance, invalid); });
        rejects<std::out_of_range>([&] { Dijkstra(graph, distance, invalid); });
        rejects<std::out_of_range>([&] { Dijkstra(2, Input{}, distance, 0, invalid, path); });
        rejects<std::out_of_range>([&] { Dijkstra(graph, distance, 0, invalid, path); });
    }
    // 従来版は到達した行だけを検査する。未到達行まで走査しない。
    WGraph unreachable(3);
    unreachable[1].emplace_back(7, -1);
    Dijkstra(unreachable, distance, 0);
    require(distance == std::vector<std::int64_t>({0, kInf, kInf}));
    Dijkstra(unreachable, distance, 0, 2, path);
    require(distance == std::vector<std::int64_t>({0, kInf, kInf}) && path.empty());
    for (Edge invalid : {Edge{3, 0}, Edge{1, -1}, Edge{1, 1'000'000'001}}) {
        WGraph reached(3);
        reached[0].push_back(invalid);
        rejects<std::out_of_range>([&] { Dijkstra(reached, distance, 0); });
        rejects<std::out_of_range>([&] { Dijkstra(reached, distance, 0, 0, path); });
    }
#endif
    Input mutable_edges{{0, 1, 3}, {1, 2, 5}};
    const auto original = mutable_edges;
    Dijkstra(3, mutable_edges, distance, 0, 2, path);
    require(distance == std::vector<std::int64_t>({0, 3, 8}));
    require(path == std::vector<int>({0, 1, 2}) && mutable_edges == original);
}
} // namespace

int main() {
    std::mt19937_64 random(0xD1A5C5A20261113ULL);
    // CIでは小さい乱択例を使い、各カーネルの境界を別に固定して確認する。
    for (int round = 0; round < 128; ++round) {
        const int n = 1 + random() % 40;
        Input edges;
        for (int i = 0, m = random() % (5 * n + 1); i < m; ++i) {
            const std::int64_t weight = round % 4 == 0 ? 0 : round % 4 == 1 ? 1 :
                round % 4 == 2 ? 1'000'000'000 : std::int64_t(random() % 1'000'000'001);
            const int u = random() % n, v = random() % n;
            edges.emplace_back(u, v, weight);
        }
        const int source = random() % n, target = random() % n;
        inspect(n, edges, source, target, true);
    }
    for (int n : {1, 2, 255, 256, 257, 4095, 4096, 4097}) {
        Input chain;
        for (int i = 1; i < n; ++i) chain.emplace_back(i - 1, i, 1'000'000'000);
        inspect(n, chain, 0, n - 1, n <= 257);
        inspect(n, chain, n - 1, 0, n <= 257);
        Input star;
        for (int i = 1; i < n; ++i) {
            star.emplace_back(0, i, i % 2 ? 0 : 1'000'000'000);
            star.emplace_back(0, i, 1'000'000'000);
        }
        inspect(n, star, 0, n - 1, n <= 257);
        Input mixed;
        for (int i = 0; i < 3 * n; ++i) {
            const int u = random() % n, v = random() % n;
            const std::int64_t weight = random() % 1'000'000'001;
            mixed.emplace_back(u, v, weight);
        }
        const int source = random() % n, target = random() % n;
        inspect(n, mixed, source, target, n <= 257);
        inspect(n, Input{}, n - 1, 0, n <= 257);
    }
    for (int n : {257, 4097, 20000}) {
        Input chain{{0, 1, 0}};
        for (int i = 2; i < n; ++i) chain.emplace_back(i - 1, i, 1'000'000'000);
        inspect(n, chain, 0, 1, false);
    }
    input_contracts();
    std::printf("PASS Dijkstra bulk/WGraph Bellman-Ford/heap contracts: cases=%ju checks=%ju\n",
                static_cast<std::uintmax_t>(cases), static_cast<std::uintmax_t>(checks));
}
