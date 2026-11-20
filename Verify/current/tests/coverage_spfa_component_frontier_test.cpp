// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Bellman-Ford_SFPA.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

namespace {

void resume_one_component(bool negative_cycle) {
    constexpr int kLast = 32, kTail = kLast + 1, kUnreachable = kLast + 2;
    sirkit::WGraph graph(kUnreachable + 1);
    graph[0].emplace_back(1, 7);
    for (int v = 1; v < kLast; ++v) {
        // DFSが先に見る辺は正だが、最短路は最後の平行辺を選ぶ。
        for (const int weight : {2, 1, -1}) graph[v].emplace_back(v + 1, weight);
        graph[v + 1].emplace_back(v, negative_cycle && v == 12 ? 0 : 1);
    }
    graph[kLast].emplace_back(kTail, 5);
    std::vector<std::int64_t> expected(graph.size(), sirkit::kInf);
    expected[0] = 0;
    for (int v = 1; v <= kLast; ++v)
        expected[v] = negative_cycle ? -sirkit::kInf : 8 - v;
    expected[kTail] = negative_cycle ? -sirkit::kInf : expected[kLast] + 5;

    // 0を処理した直後から再開し、一度に1層ずつ有限距離を広げる。
    std::vector<std::int64_t> distance(graph.size(), sirkit::kInf);
    distance[0] = 0;
    distance[1] = 7;
    std::vector<std::uint8_t> state(graph.size());
    std::vector<int> affected;
    sirkit::detail::SPFAFinishComponents(graph, distance, sirkit::kInf, affected, state);
    assert(distance == expected);
    assert(affected.size() == (negative_cycle ? kTail : 0));
    assert(sirkit::SPFA(graph, distance, 0) == negative_cycle);
    assert(distance == expected);
}

void resume_near_lower_bound(int cycle_size, std::int64_t weight) {
    constexpr int kVertexCount = 65;
    constexpr std::int64_t kMaxCost = 1'000'000'000;
    const int prefix_size = kVertexCount - cycle_size;
    sirkit::WGraph graph(kVertexCount);
    std::vector<std::int64_t> distance(kVertexCount);
    for (int v = 0; v < prefix_size; ++v) {
        graph[v].emplace_back(v + 1, -kMaxCost);
        distance[v] = -v * kMaxCost;
    }
    for (int v = prefix_size; v + 1 < kVertexCount; ++v) {
        // DFSが使う最初の辺では、閉路の重みは0になる。
        graph[v].emplace_back(v + 1, cycle_size == 2 ? kMaxCost : 0);
        graph[v].emplace_back(v + 1, -weight);
    }
    graph.back().emplace_back(prefix_size, cycle_size == 2 ? -kMaxCost : 0);
    // FIFOで閉路の最後の頂点に初めて到達した直後の距離。
    for (int v = prefix_size; v < kVertexCount; ++v)
        distance[v] = -prefix_size * kMaxCost - (v - prefix_size) * weight;
    auto expected = distance;
    for (int v = prefix_size; v < kVertexCount; ++v)
        expected[v] = -sirkit::kInf;

    std::vector<std::uint8_t> state(kVertexCount);
    std::vector<int> affected;
    sirkit::detail::SPFAFinishComponents(graph, distance, sirkit::kInf, affected, state);
    assert(distance == expected && affected.size() == static_cast<std::size_t>(cycle_size));
    assert(sirkit::SPFA(graph, distance, 0));
    assert(distance == expected);
}

} // namespace

int main() {
    resume_one_component(false);
    resume_one_component(true);
    // 前半の緩和と、8回目からの緩和でそれぞれ距離下限へ達する。
    resume_near_lower_bound(2, 1'000'000'000);
    resume_near_lower_bound(9, 580'000'000);
}
