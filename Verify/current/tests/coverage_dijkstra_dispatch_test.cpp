// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Dijkstra.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace {

using Distance = std::int64_t;
using Edges = std::vector<std::tuple<int, int, Distance>>;

void check_distances(const Edges& edges, const std::vector<Distance>& expected,
                     int start, int target, const std::vector<int>& expected_path) {
    const int size = static_cast<int>(expected.size());
    sirkit::WGraph graph(expected.size());
    for (auto [from, to, weight] : edges) graph[from].push_back({to, weight});
    std::vector<Distance> distance{-1};
    std::vector<int> path{-1};
    sirkit::Dijkstra(graph, distance, start);
    assert(distance == expected);
    sirkit::Dijkstra(graph, distance, start, target, path);
    assert(distance == expected && path == expected_path);
    sirkit::Dijkstra(size, edges, distance, start);
    assert(distance == expected);
    sirkit::Dijkstra(size, edges, distance, start, target, path);
    assert(distance == expected && path == expected_path);
}

template<class Error, class F>
void expect_exception(F operation) {
    try {
        operation();
    } catch (const Error&) {
        return;
    }
    assert(false);
}

void cycle_and_unreachable_target() {
    constexpr int kSize = 4'097;
    const Edges edges{{2, 6, 4}, {6, 8, 0}, {8, 6, 0}};
    std::vector<Distance> expected(kSize, sirkit::kInf);
    expected[2] = 0;
    expected[6] = expected[8] = 4;
    // 一本道の先は既に到達した頂点に戻る。到達不能の目的地は空経路。
    check_distances(edges, expected, 2, kSize - 1, {});
    check_distances(edges, expected, 2, 8, {2, 6, 8});
}

void equal_priority_and_checkpoints() {
    // 同距離の大きい頂点番号を先に加える。後半は既に確定した頂点へ戻る。
    const Edges edges{{5, 6, 2}, {6, 100, 3}, {6, 7, 3}, {7, 6, 0},
                      {100, 105, 9}, {105, 100, 0}};
    for (int size : {256, 257, 4'096, 4'097}) {
        std::vector<Distance> expected(size, sirkit::kInf);
        expected[5] = 0;
        expected[6] = 2;
        expected[7] = expected[100] = 5;
        expected[105] = 14;
        check_distances(edges, expected, 5, 105, {5, 6, 100, 105});
    }
}

void invalid_arguments() {
    constexpr int kSize = 4'097;
    sirkit::WGraph graph(kSize);
    graph[0].push_back({1, 0});
    std::vector<Distance> distance{-1};
    std::vector<int> path{-1};
    expect_exception<std::invalid_argument>([&] { sirkit::Dijkstra(graph, distance, 0, 0); });
#ifndef SKIP_RUNTIME_CHECKS
    expect_exception<std::out_of_range>([&] { sirkit::Dijkstra(graph, distance, kSize, 0, path); });
    expect_exception<std::out_of_range>([&] { sirkit::Dijkstra(graph, distance, 0, -1, path); });
    expect_exception<std::out_of_range>([&] { sirkit::Dijkstra(graph, distance, 0, -1, path, 17); });
    sirkit::WGraph small(2);
    small[0].push_back({-1, 0});
    expect_exception<std::out_of_range>([&] { sirkit::Dijkstra(small, distance, 0, 1, path); });
#endif
    assert(distance == std::vector<Distance>{-1} && path == std::vector<int>{-1});
}

class OversizedAdjacency {
public:
    std::size_t size() const {
        return static_cast<std::size_t>(std::numeric_limits<int>::max()) + 1;
    }
    const std::vector<sirkit::Edge>& operator[](int) const { std::abort(); }
    const std::vector<sirkit::Edge>* data() const { std::abort(); }
};

void rejects_oversized_adjacency() {
    // 各内部実装が隣接リストの参照前に頂点数を拒否することを確認する。
    OversizedAdjacency graph;
    std::vector<Distance> distance{-1};
    expect_exception<std::length_error>([&] {
        sirkit::detail::DijkstraImpl<false>(graph, distance, 0, -1, nullptr);
    });
    expect_exception<std::length_error>([&] {
        sirkit::detail::DijkstraLarge<false, true, false>(graph, distance, 0, -1, nullptr);
    });
    expect_exception<std::length_error>([&] {
        sirkit::detail::DijkstraCustomInf<false>(graph, distance, 0, -1, nullptr, 17);
    });
    assert(distance == std::vector<Distance>{-1});
}

template <bool PruneTerminal>
void conservative_distance_bound(int size) {
    // 距離が番兵を超えないと事前に仮定しない経路も、独立に求めた最短距離と一致する。
    sirkit::WGraph graph(size);
    graph[0] = {{1, 4}};
    graph[1] = {{2, 3}, {3, 8}};
    graph[2] = {{3, 2}};
    graph[3] = {{4, 0}};
    std::vector<Distance> expected(size, sirkit::kInf);
    expected[0] = 0;
    expected[1] = 4;
    expected[2] = 7;
    expected[3] = expected[4] = 9;
    std::vector<Distance> actual;
    std::vector<int> path;
    sirkit::detail::DijkstraLarge<true, true, PruneTerminal>(graph, actual, 0, 4, &path);
    assert(actual == expected);
    assert((path == std::vector<int>{0, 1, 2, 3, 4}));

    expected.assign(size, sirkit::kInf);
    expected[4] = 0;
    sirkit::detail::DijkstraLarge<true, true, PruneTerminal>(graph, actual, 4, 4, &path);
    assert(actual == expected && path == std::vector<int>{4});
}

}  // namespace

int main() {
    cycle_and_unreachable_target();
    equal_priority_and_checkpoints();
    invalid_arguments();
    rejects_oversized_adjacency();
    conservative_distance_bound<false>(257);
    conservative_distance_bound<true>(257);
    conservative_distance_bound<true>(4'097);
}
