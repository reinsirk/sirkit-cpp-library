// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Bellman-Ford.cpp"
#include "../../../Graph/Bellman-Ford_SFPA.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <stdexcept>
#include <vector>

namespace {

using Distance = std::int64_t;

void capacity_and_component_dispatch() {
    constexpr auto kLimit = static_cast<std::size_t>(std::numeric_limits<int>::max());
    assert(sirkit::detail::BellmanFordVertexCount(kLimit) == std::numeric_limits<int>::max());
    bool rejected = false;
    try {
        (void)sirkit::detail::BellmanFordVertexCount(kLimit + 1);
    } catch (const std::length_error&) {
        rejected = true;
    }
    assert(rejected);

    sirkit::WGraph graph(80);
    std::vector<int> frontier(64);
    std::iota(frontier.begin(), frontier.end(), 0);
    assert(sirkit::detail::BellmanFordNeedsComponents(graph, frontier));
    frontier.pop_back();
    assert(!sirkit::detail::BellmanFordNeedsComponents(graph, frontier));
}

void sparse_negative_self_loop() {
    // 自己辺を先に処理すると、終点は緩和済みで負影響の種にならない。
    // 負影響の伝播が残った1頂点を最後のパスで更新する。
    sirkit::WGraph graph(4'096);
    graph[0] = {{0, -1}, {1, 0}};
    std::vector<Distance> expected(graph.size(), sirkit::kInf), distance;
    expected[0] = expected[1] = -sirkit::kInf;
    const bool negative = sirkit::BellmanFord(graph, distance, 0);
    assert(negative && distance == expected);
}

void descending_chain() {
    constexpr int kSize = 129;
    sirkit::WGraph graph(kSize);
    std::vector<Distance> expected(kSize), distance;
    for (int i = 1; i < kSize; ++i)
        graph[i].emplace_back(i - 1, -1);
    for (int i = 0; i < kSize; ++i)
        expected[i] = i - (kSize - 1);
    const bool negative = sirkit::BellmanFord(graph, distance, kSize - 1);
    assert(!negative && distance == expected);
#ifndef SKIP_RUNTIME_CHECKS
    // 初期2パスより後で到達する行を、続きの緩和で検査する。
    for (auto invalid : {sirkit::Edge{-1, 0}, sirkit::Edge{0, -1'000'000'001}}) {
        graph[kSize - 4] = {invalid};
        bool rejected = false;
        try {
            (void)sirkit::BellmanFord(graph, distance, kSize - 1);
        } catch (const std::out_of_range&) {
            rejected = true;
        }
        assert(rejected && distance == expected);
    }
#endif
}

void finish_components() {
    constexpr auto kInf = sirkit::kInf;
    // FIFOからの引継ぎ時の距離を与え、非負環と経路に戻り辺がある成分を照合する。
    // 費用は 0→1 が10、1→2 が-2、2→3 が-1、3→1 が3。
    for (bool ring : {false, true}) {
        sirkit::WGraph graph(6);
        graph[0] = {{1, 10}};
        graph[1] = {{2, -2}};
        graph[2] = {{3, -1}};
        graph[3] = {{1, 3}};
        if (!ring)
            graph[2].emplace_back(4, 5);
        std::vector<Distance> distance{0, 10, kInf, kInf, kInf, kInf};
        std::vector<std::uint8_t> state(graph.size());
        std::vector<int> affected;
        sirkit::detail::SPFAFinishComponents(graph, distance, kInf, affected, state);
        const std::vector<Distance> expected{0, 10, 8, 7, ring ? kInf : 13, kInf};
        assert(distance == expected && affected.empty());
    }

    // 2はDFSで先に調べ終わるため、3→2を祖先への戻り辺としては検出できない。
    sirkit::WGraph graph(6);
    graph[0] = {{1, 10}};
    graph[1] = {{2, 1}, {3, 0}};
    graph[2] = {{1, 0}, {4, 0}};
    graph[3] = {{2, -2}};
    std::vector<Distance> distance{0, 10, kInf, kInf, kInf, kInf};
    std::vector<std::uint8_t> state(graph.size());
    std::vector<int> affected;
    sirkit::detail::SPFAFinishComponents(graph, distance, kInf, affected, state);
    const std::vector<Distance> expected{0, -kInf, -kInf, -kInf, -kInf, kInf};
    assert(distance == expected && affected.size() == 4);
}

}  // namespace

int main() {
    capacity_and_component_dispatch();
    sparse_negative_self_loop();
    descending_chain();
    finish_components();
}
