// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Dijkstra.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace {

using Distance = std::int64_t;
using Edges = std::vector<std::tuple<int, int, Distance>>;
using sirkit::Dijkstra;
using sirkit::WGraph;
using sirkit::kInf;

WGraph adjacency(int size, const Edges& edges) {
    WGraph graph(size);
    for (const auto& [from, to, weight] : edges) graph[from].push_back({to, weight});
    return graph;
}

std::vector<Distance> reference(int size, const Edges& edges, Distance inf) {
    std::vector<Distance> distance(size, kInf);
    distance[0] = 0;
    for (int pass = 1; pass < size; ++pass) {
        bool changed = false;
        for (const auto& [from, to, weight] : edges) {
            if (distance[from] == kInf || distance[to] <= distance[from] + weight) continue;
            distance[to] = distance[from] + weight;
            changed = true;
        }
        if (!changed) break;
    }
    std::replace(distance.begin(), distance.end(), kInf, inf);
    return distance;
}

void check_all(int size, const Edges& edges, int target, const std::vector<int>& expected_path,
               Distance inf = kInf) {
    const auto graph = adjacency(size, edges);
    const auto expected = reference(size, edges, inf);
    std::vector<Distance> distance{-11};
    std::vector<int> path{-13};
    Dijkstra(graph, distance, 0, inf);
    assert(distance == expected);
    Dijkstra(graph, distance, 0, target, path, inf);
    assert(distance == expected && path == expected_path);
    Dijkstra(size, edges, distance, 0, inf);
    assert(distance == expected);
    Dijkstra(size, edges, distance, 0, target, path, inf);
    assert(distance == expected && path == expected_path);
}

template<class Error, class Action>
void expect_rejection(Action action) {
    bool rejected = false;
    try {
        action();
    } catch (const Error&) {
        rejected = true;
    }
    assert(rejected);
}

void custom_inf() {
    constexpr Distance kSmallInf = 12;
    // 番兵以上になる候補の先に別の短い経路が見つかる。
    const Edges edges{{0, 1, 12}, {0, 2, 2}, {0, 3, 10}, {2, 3, 2},
                      {3, 1, 1}, {1, 2, 9}, {1, 4, 6}, {4, 4, 0}};
    check_all(6, edges, 4, {0, 2, 3, 1, 4}, kSmallInf);
    check_all(6, edges, 5, {}, kSmallInf);
    check_all(6, edges, 0, {0}, kSmallInf);

    std::vector<Distance> distance{-11};
    std::vector<int> path{-13};
    for (Distance last_weight : {10, 11}) {
        const Edges distant{{0, 1, 2}, {1, 2, last_weight}};
        const auto graph = adjacency(3, distant);
        // 目的地へ到達できても、別頂点の最短距離が番兵以上なら失敗する。
        expect_rejection<std::overflow_error>([&] { Dijkstra(graph, distance, 0, kSmallInf); });
        expect_rejection<std::overflow_error>([&] { Dijkstra(graph, distance, 0, 1, path, kSmallInf); });
        expect_rejection<std::overflow_error>([&] { Dijkstra(3, distant, distance, 0, kSmallInf); });
        expect_rejection<std::overflow_error>([&] { Dijkstra(3, distant, distance, 0, 1, path, kSmallInf); });
    }
    const auto graph = adjacency(6, edges);
    expect_rejection<std::invalid_argument>([&] { Dijkstra(graph, distance, 0, 4, path, 0); });
    expect_rejection<std::invalid_argument>([&] { Dijkstra(6, edges, distance, 0, -1); });
    expect_rejection<std::invalid_argument>([&] { Dijkstra(6, edges, distance, 0, 4, path, 0); });
#ifndef SKIP_RUNTIME_CHECKS
    for (int index : {-1, 6}) {
        expect_rejection<std::out_of_range>([&] { Dijkstra(graph, distance, index, kSmallInf); });
        expect_rejection<std::out_of_range>([&] { Dijkstra(graph, distance, 0, index, path, kSmallInf); });
        expect_rejection<std::out_of_range>([&] { Dijkstra(6, edges, distance, index, kSmallInf); });
        expect_rejection<std::out_of_range>([&] { Dijkstra(6, edges, distance, 0, index, path, kSmallInf); });
    }
    for (const auto& invalid : {sirkit::Edge{-1, 0}, sirkit::Edge{6, 0}, sirkit::Edge{1, -1},
                                sirkit::Edge{1, 1'000'000'001}}) {
        auto copy = graph;
        copy[4].push_back(invalid);
        expect_rejection<std::out_of_range>([&] { Dijkstra(copy, distance, 0, kSmallInf); });
        expect_rejection<std::out_of_range>([&] { Dijkstra(copy, distance, 0, 1, path, kSmallInf); });
    }
    assert(distance == std::vector<Distance>{-11} && path == std::vector<int>{-13});
#endif
}

Edges large_fan() {
    Edges edges{{0, 1, 5}};
    // 幅の広い分岐、大きな重み、合流を混ぜる。
    for (int to = 2; to < 154; ++to) {
        edges.emplace_back(1, to, to);
        edges.emplace_back(to, 154, 1'000'000'000);
        edges.emplace_back(to, 1, 0);
    }
    edges.emplace_back(154, 155, 1'000'000'000);
    edges.emplace_back(155, 156, 1'000'000'000);
    edges.emplace_back(156, 157, 1'000'000'000);
    edges.emplace_back(157, 158, 1'000'000'000);
    return edges;
}

void large_paths() {
    constexpr int kSize = 4'097;
    const Edges short_tail{{0, 1, 1}, {1, 2, 2}, {1, 3, 4}, {2, 3, 1}, {3, 1, 0}};
    check_all(kSize, short_tail, 3, {0, 1, 2, 3});
    check_all(kSize, short_tail, kSize - 1, {});

    Edges long_tail{{0, 1, 1}};
    std::vector<int> long_path{0, 1};
    for (int from = 1; from <= 80; ++from) {
        long_tail.emplace_back(from, from + 1, 1);
        long_tail.emplace_back(from, from - 1, 0);
        long_path.push_back(from + 1);
    }
    check_all(kSize, long_tail, 81, long_path);
    check_all(kSize, large_fan(), 158, {0, 1, 2, 154, 155, 156, 157, 158});

#ifndef SKIP_RUNTIME_CHECKS
    auto graph = adjacency(kSize, large_fan());
    std::vector<Distance> distance{-11};
    std::vector<int> path{-13};
    for (const auto& invalid : {sirkit::Edge{-1, 0}, sirkit::Edge{kSize, 0},
                                sirkit::Edge{1, -1}, sirkit::Edge{1, 1'000'000'001}}) {
        // 最初の一本道、分岐の途中、離れた到達頂点で不正辺を検出する。
        for (const int from : {0, 1, 2, 80}) {
            graph[from].push_back(invalid);
            expect_rejection<std::out_of_range>([&] { Dijkstra(graph, distance, 0); });
            expect_rejection<std::out_of_range>([&] { Dijkstra(graph, distance, 0, 0, path); });
            assert(distance == std::vector<Distance>{-11} && path == std::vector<int>{-13});
            graph[from].pop_back();
        }
        auto chain = graph;
        chain[0].front() = invalid;
        expect_rejection<std::out_of_range>([&] { Dijkstra(chain, distance, 0, 0, path); });
        auto early_branch = adjacency(kSize, short_tail);
        early_branch[1].push_back(invalid);
        expect_rejection<std::out_of_range>([&] { Dijkstra(early_branch, distance, 0, 0, path); });
        auto after_merge = adjacency(kSize, Edges{{0, 1, 1}, {1, 2, 1}, {1, 2, 2}});
        after_merge[2].push_back(invalid);
        expect_rejection<std::out_of_range>([&] { Dijkstra(after_merge, distance, 0, 0, path); });
        auto during_tail_pruning = adjacency(kSize, Edges{{0, 1, 5}});
        for (int to = 2; to < 75; ++to) during_tail_pruning[1].push_back({to, to});
        during_tail_pruning[68].push_back(invalid);
        expect_rejection<std::out_of_range>([&] { Dijkstra(during_tail_pruning, distance, 0); });
        expect_rejection<std::out_of_range>([&] { Dijkstra(during_tail_pruning, distance, 0, 0, path); });
    }
    expect_rejection<std::out_of_range>([&] { Dijkstra(graph, distance, -1, 0, path); });
    expect_rejection<std::out_of_range>([&] { Dijkstra(graph, distance, 0, kSize, path); });
    assert(distance == std::vector<Distance>{-11} && path == std::vector<int>{-13});
#endif
}

} // namespace

int main() {
    custom_inf();
    large_paths();
}
