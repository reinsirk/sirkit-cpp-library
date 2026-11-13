// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TEST_TOPOLOGICAL_SOURCE
#define SIRKIT_TEST_TOPOLOGICAL_SOURCE "../../../Graph/Topological.cpp"
#endif
#include SIRKIT_TEST_TOPOLOGICAL_SOURCE
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <deque>
#include <numeric>
#include <random>
#include <vector>
static std::uintmax_t cases = 0, checks = 0;
static void require(bool value) { ++checks; if (!value) std::abort(); }
// 入次数による分岐省略をしない、dequeを使う通常のKahn参照。
static TopologicalResult reference(const Graph& graph) {
    std::vector<std::size_t> degree(graph.size());
    for (const auto& row : graph) for (int to : row) ++degree[static_cast<std::size_t>(to)];
    std::deque<int> ready;
    for (std::size_t i = 0; i < graph.size(); ++i) if (degree[i] == 0) ready.push_back(static_cast<int>(i));
    TopologicalResult result; result.position.assign(graph.size(), -1);
    while (!ready.empty()) {
        if (ready.size() > 1) result.unique = false;
        const int from = ready.front(); ready.pop_front();
        result.position[static_cast<std::size_t>(from)] = static_cast<int>(result.order.size());
        result.order.push_back(from);
        for (int to : graph[static_cast<std::size_t>(from)]) if (--degree[static_cast<std::size_t>(to)] == 0) ready.push_back(to);
    }
    result.acyclic = result.order.size() == graph.size(); result.unique &= result.acyclic;
    return result;
}
static void check(const Graph& graph) {
    ++cases; const auto saved = graph; const auto expected = reference(graph); const auto actual = TopologicalSortDetails(graph);
    require(actual.order == expected.order); require(actual.position == expected.position);
    require(actual.acyclic == expected.acyclic); require(actual.unique == expected.unique);
    require(TopologicalSort(graph) == (expected.acyclic ? expected.order : std::vector<int>{}));
    require(IsDAG(graph) == expected.acyclic);
    // 小さい全列挙でも、大規模用の分岐を直接選んで独立参照へ照合する。
    const auto bulk = reinsirk::detail::TopologicalRun<false, false, true>(graph);
    require(bulk.acyclic == expected.acyclic);
    require(bulk.order == (expected.acyclic ? expected.order : std::vector<int>{}));
    require(graph == saved);
}
int main() {
    // 各頂点の唯一の親またはrootを全列挙し、自己閉路・未到達閉路・外向き木も含める。
    for (std::size_t n = 0; n <= 6; ++n) {
        std::size_t combinations = 1;
        for (std::size_t i = 0; i < n; ++i) combinations *= n + 1;
        for (std::size_t mask = 0; mask < combinations; ++mask) {
            Graph graph(n); auto code = mask;
            for (std::size_t to = 0; to < n; ++to) { const auto from = code % (n + 1); code /= n + 1; if (from < n) graph[from].push_back(static_cast<int>(to)); }
            check(graph);
        }
    }
    std::mt19937 rng(0x71a91u);
    for (int n : {4095,4096,4097,65536}) for (int shape = 0; shape < 6; ++shape) {
        std::vector<int> labels(static_cast<std::size_t>(n)); std::iota(labels.begin(),labels.end(),0); std::shuffle(labels.begin(),labels.end(),rng);
        Graph graph(static_cast<std::size_t>(n));
        for (int to = 0; to < n; ++to) {
            int from = -1;
            if (shape == 1 && to != 0) from = to - 1;
            if (shape == 2 && to != 0) from = 0;
            if (shape == 3) from = (to + n - 1) % n;
            if (shape == 4) {
                const int half = n / 2;
                if (to > 0 && to < half) from = to - 1;
                if (to == half) from = half + 1;
                if (to == half + 1) from = half;
                if (to > half + 1) from = half + (to - half - 2) / 2;
            }
            if (shape == 5) { from = static_cast<int>(rng() % (static_cast<unsigned>(n) + 1)); if (from == n) from = -1; }
            if (from >= 0) graph[static_cast<std::size_t>(labels[static_cast<std::size_t>(from)])].push_back(labels[static_cast<std::size_t>(to)]);
        }
        check(graph);
        // 追加辺で判定が変わる場合も、通常参照と比較する。
        graph[static_cast<std::size_t>(labels[0])].push_back(labels[1]); check(graph);
    }
    std::printf("single incoming reference cases=%ju checks=%ju\n",cases,checks);
}
