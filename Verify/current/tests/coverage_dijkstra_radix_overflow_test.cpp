// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Dijkstra.cpp"

#include <cassert>
#include <cstdint>
#include <utility>
#include <vector>

namespace {

template<bool RestorePath>
void near_default_inf() {
    constexpr auto kInf = sirkit::kInf;
    constexpr auto kPrefixDistance = kInf - 10;
    sirkit::WGraph graph(11);
    graph[0] = {{1, 2}, {3, 10}, {4, 11}, {5, 1}, {8, 0}, {9, 6}};
    graph[1] = {{2, 8}};
    graph[5] = {{4, 9}};
    graph[8] = {{7, 7}};
    graph[10] = {{4, 9}};

    // 長い経路の残りだけを取り出し、上限直前のradix緩和を検証する。
    // 4は先行の緩和で有限距離が確定した終端。
    std::vector<std::int64_t> distance(graph.size(), kInf);
    distance[0] = distance[10] = kPrefixDistance;
    distance[4] = kPrefixDistance + 9;
    std::vector<int> previous(graph.size(), -1);
    previous[4] = 10;
    std::vector<int> overflow_targets;
    sirkit::detail::DijkstraRadixHeap queue(kPrefixDistance);
    sirkit::detail::DijkstraRadixRun<RestorePath, true>(
        graph, distance, previous, overflow_targets, queue, 0, kPrefixDistance);

    // 2,3はあふれ、4は既存の有限値を保持。7の2辺は有限。
    const std::vector<std::int64_t> expected{
        kPrefixDistance, kPrefixDistance + 2, kInf, kInf, kPrefixDistance + 9,
        kPrefixDistance + 1, kInf, kPrefixDistance + 7, kPrefixDistance,
        kPrefixDistance + 6, kPrefixDistance,
    };
    assert(queue.empty() && distance == expected);
    assert((overflow_targets == std::vector<int>{2, 3}));
    if constexpr (RestorePath)
        assert((previous == std::vector<int>{-1, 0, -1, -1, 10, 0, -1, 8, 0, 0, -1}));
    else
        assert((previous == std::vector<int>{-1, -1, -1, -1, 10, -1, -1, -1, -1, -1, -1}));
}

void after_terminal_pruning() {
    constexpr auto kInf = sirkit::kInf;
    constexpr auto kPrefixDistance = kInf - 10;
    // 65件を登録した後、終端でない行先がさらに64件続く。
    sirkit::WGraph graph(4'097);
    for (int i = 1; i <= 129; ++i) {
        graph[0].emplace_back(i, 0);
        graph[i].emplace_back(130, 0);
    }
    graph[130] = {{131, 0}, {132, 10}, {133, 11}};
    graph[134] = {{133, 9}};

    std::vector<std::int64_t> distance(graph.size(), kInf);
    std::vector<int> previous(graph.size(), -1);
    std::vector<std::pair<std::int64_t, int>> initial;
    distance[0] = distance[134] = kPrefixDistance;
    distance[133] = kPrefixDistance + 9;
    previous[133] = 134;
    for (int i = 1; i <= 65; ++i) {
        initial.emplace_back(kPrefixDistance, i);
        distance[i] = kPrefixDistance;
        previous[i] = 0;
    }
    std::vector<int> overflow_targets;
    sirkit::detail::DijkstraRadixFinish<true, true>(
        graph, distance, previous, overflow_targets, initial, kPrefixDistance, 0, 65);

    auto expected = std::vector<std::int64_t>(graph.size(), kInf);
    for (int i = 0; i <= 131; ++i)
        expected[i] = kPrefixDistance;
    expected[133] = kPrefixDistance + 9;
    expected[134] = kPrefixDistance;
    assert(distance == expected && overflow_targets == std::vector<int>{132});
    assert(1 <= previous[130] && previous[130] <= 129);
    assert(previous[131] == 130 && previous[133] == 134);
    for (int i = 1; i <= 129; ++i)
        assert(previous[i] == 0);
}

void resume_small_frontier() {
    const sirkit::WGraph graph{{{1, 1}, {2, 3}}, {{3, 7}}, {{3, 2}}, {}};
    std::vector<int> overflow_targets;
    for (const bool row_complete : {false, true}) {
        // 頂点0の出辺を1本または2本処理した直後の状態。
        auto distance = std::vector<std::int64_t>{0, 1, sirkit::kInf, sirkit::kInf};
        auto previous = std::vector<int>{-1, 0, -1, -1};
        auto frontier = std::vector<std::pair<std::int64_t, int>>{{1, 1}};
        if (row_complete) {
            distance[2] = 3;
            previous[2] = 0;
            frontier.emplace_back(3, 2);
        }
        sirkit::detail::DijkstraRadixFinish<true, false>(
            graph, distance, previous, overflow_targets, frontier, 0, 0, row_complete ? 2 : 1);
        assert((distance == std::vector<std::int64_t>{0, 1, 3, 5}));
        assert((previous == std::vector<int>{-1, 0, 0, 2}));
        assert(overflow_targets.empty());
    }

    // 0の行は処理済みなので、再処理する頂点を渡さずradixキューから続ける。
    auto distance = std::vector<std::int64_t>{0, 1, 3, sirkit::kInf};
    auto previous = std::vector<int>{-1, 0, 0, -1};
    sirkit::detail::DijkstraRadixHeap frontier(0);
    frontier.emplace(1, 1);
    frontier.emplace(3, 2);
    sirkit::detail::DijkstraRadixRun<true, false>(graph, distance, previous, overflow_targets, frontier);
    assert((distance == std::vector<std::int64_t>{0, 1, 3, 5}));
    assert((previous == std::vector<int>{-1, 0, 0, 2}));
    assert(overflow_targets.empty());
}

}  // namespace

int main() {
    near_default_inf<true>();
    near_default_inf<false>();
    after_terminal_pruning();
    resume_small_frontier();
}
