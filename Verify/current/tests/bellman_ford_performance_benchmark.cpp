// SPDX-License-Identifier: CC0-1.0
// 入力生成・ファイル読込を除き、同じグラフへのBellmanFord呼出しだけを計測する。
#ifndef SIRKIT_TEST_NEGATIVE_PATH_SOURCE
#define SIRKIT_TEST_NEGATIVE_PATH_SOURCE "../../../Graph/Bellman-Ford.cpp"
#endif
#include SIRKIT_TEST_NEGATIVE_PATH_SOURCE
#include <algorithm>
#include <chrono>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <numeric>
#include <random>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    if (argc != 5) return 2;
    const std::string kind = argv[1];
    int n = std::stoi(argv[2]);
    const int repetitions = std::stoi(argv[3]);
    int start = 0;
    WGraph graph(n);
    std::mt19937 rng(20261025);
    if (kind == "file") {
        std::ifstream input(argv[4]);
        int m{}; std::int64_t price{};
        if (!(input >> n >> m >> price)) return 3;
        graph.resize(n);
        while (m--) {
            int a{}, b{}; std::int64_t coins{};
            input >> a >> b >> coins;
            graph[a - 1].emplace_back(b - 1, price - coins);
        }
    } else if (kind == "forward" || kind == "reverse") {
        for (int i = 1; i < n; ++i) {
            if (kind == "forward") graph[i - 1].emplace_back(i, -1);
            else graph[i].emplace_back(i - 1, -1);
        }
        if (kind == "reverse") start = n - 1;
    } else if (kind == "star") {
        for (int i = 1; i < n; ++i) graph[0].emplace_back(i, i % 19);
    } else if (kind == "permuted" || kind == "sparse_chain") {
        const int count = kind == "sparse_chain" ? std::min(n, 200) : n;
        std::vector<int> order(count);
        std::iota(order.begin(), order.end(), 0);
        std::shuffle(order.begin(), order.end(), rng);
        start = order.front();
        for (int i = 1; i < count; ++i) graph[order[i - 1]].emplace_back(order[i], -1);
    } else if (kind == "zero_ring" || kind == "zero_ring_exit") {
        const int count = kind == "zero_ring_exit" ? n - 1 : n;
        std::vector<int> order(count);
        std::iota(order.begin(), order.end(), 0);
        std::shuffle(order.begin(), order.end(), rng);
        start = order.front();
        for (int i = 0; i < count; ++i) graph[order[i]].emplace_back(order[(i + 1) % count], 0);
        if (kind == "zero_ring_exit") graph[order.back()].emplace_back(n - 1, 0);
    } else if (kind == "random" || kind == "negative" || kind == "negative_no_self" || kind == "potential") {
        std::vector<std::int64_t> potential(n);
        for (auto& x : potential) x = static_cast<int>(rng() % 2001) - 1000;
        for (int from = 0; from < n; ++from) for (int j = 0; j < 8; ++j) {
            int to = static_cast<int>(rng() % n);
            if (kind == "negative_no_self" && to == from) to = (to + 1) % n;
            const std::int64_t weight = kind == "negative" || kind == "negative_no_self" ? static_cast<int>(rng() % 11) - 5 :
                kind == "potential" ? std::int64_t(rng() % 2) + potential[to] - potential[from] :
                std::int64_t(rng() % 20);
            graph[from].emplace_back(to, weight);
        }
    } else if (kind == "components" || kind == "negative_components") {
        std::vector<int> order(n);
        std::iota(order.begin(), order.end(), 0);
        std::shuffle(order.begin(), order.end(), rng);
        start = order.front();
        for (int i = 0; i < n; ++i) {
            const int group = i / 4, begin = group * 4, end = std::min(n, begin + 4);
            graph[order[i]].emplace_back(order[i + 1 == end ? begin : i + 1],
                kind == "negative_components" && group == n / 8 ? -1 : 0);
            if (i + 4 < n) graph[order[i]].emplace_back(order[i + 4], 1);
        }
    } else if (kind != "isolated") return 4;
    std::uint64_t checksum = 0;
    std::vector<std::int64_t> distances;
    const auto begin = std::chrono::steady_clock::now();
    for (int repeat = 0; repeat < repetitions; ++repeat) {
        checksum += BellmanFord(graph, distances, start);
        checksum += static_cast<std::uint64_t>(distances.back());
    }
    const auto end = std::chrono::steady_clock::now();
    for (auto distance : distances) checksum = checksum * 1000003 + static_cast<std::uint64_t>(distance);
    std::cout << std::chrono::duration<double, std::milli>(end - begin).count()
              << ' ' << checksum << '\n';
}
