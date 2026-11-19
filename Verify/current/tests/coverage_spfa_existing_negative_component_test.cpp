// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Bellman-Ford_SFPA.cpp"

#include <cassert>
#include <cstdint>
#include <vector>

int main() {
    constexpr int kRing = 128;
    constexpr int kExit = kRing + 1;
    constexpr int kCycle = kRing + 2;
    constexpr int kTail = kRing + 3;
    constexpr int kUnreachable = kRing + 4;
    sirkit::WGraph graph(kUnreachable + 1);
    for (int vertex = 1; vertex <= kRing; ++vertex)
        graph[0].emplace_back(vertex, kRing + 1 - vertex);
    for (int vertex = 1; vertex <= kRing; ++vertex) {
        // 0重みの環と、そこから抜ける一方向の枝。
        graph[vertex].emplace_back(vertex == 1 ? kRing : vertex - 1, 0);
        graph[vertex].emplace_back(kExit, 9);
    }

    // 別の有限成分を処理中に、自己辺の負閉路とその下流が確定する。
    graph[0].emplace_back(kCycle, 0);
    graph[kCycle] = {{kCycle, -1}, {kTail, 0}};
    graph[kUnreachable] = {{kUnreachable, -1}};

    std::vector<std::int64_t> expected(graph.size(), 1), distance;
    expected[0] = 0;
    expected[kExit] = 10;
    expected[kCycle] = expected[kTail] = -sirkit::kInf;
    expected[kUnreachable] = sirkit::kInf;
    assert(sirkit::SPFA(graph, distance, 0));
    assert(distance == expected);
}
