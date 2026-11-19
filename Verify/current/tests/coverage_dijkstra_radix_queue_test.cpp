// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Dijkstra.cpp"

#include <cassert>
#include <cstdint>
#include <tuple>
#include <vector>

int main() {
    constexpr int kSize = 4'097;
    constexpr int kFan = 80;
    constexpr int kFar = 65;
    constexpr std::int64_t kFarWeight = 4'096;
    sirkit::WGraph graph(kSize);
    std::vector<std::tuple<int, int, std::int64_t>> edges;
    std::vector<std::int64_t> expected(kSize, sirkit::kInf);
    expected[0] = 0;
    // 65件でradixへ移り、空・複数件・1件のバケットを順に処理する。
    for (int to = 1; to <= kFan; ++to) {
        const std::int64_t weight = to == kFar ? kFarWeight : (to - 1) / 2 % 4;
        graph[0].emplace_back(to, weight);
        edges.emplace_back(0, to, weight);
        expected[to] = weight;
    }
    const std::vector<int> expected_path{0, kFar};
    std::vector<std::int64_t> distance;
    std::vector<int> path;
    sirkit::Dijkstra(graph, distance, 0, kFar, path);
    assert(distance == expected && path == expected_path);
    sirkit::Dijkstra(kSize, edges, distance, 0, kFar, path);
    assert(distance == expected && path == expected_path);
}
