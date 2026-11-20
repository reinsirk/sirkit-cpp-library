// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Bellman-Ford.cpp"
#include "../../../Graph/Bellman-Ford_SFPA.cpp"

#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace {

using Distance = std::int64_t;
using Solver = bool (*)(const sirkit::WGraph&, std::vector<Distance>&, int, Distance);
constexpr std::array<Solver, 2> kSolvers{sirkit::BellmanFord, sirkit::SPFA};

void check(const sirkit::WGraph& graph, int start, Distance inf,
           const std::vector<Distance>& expected, bool negative) {
    for (const auto solve : kSolvers) {
        std::vector<Distance> actual{123};
        const bool result = solve(graph, actual, start, inf);
        assert(result == negative && actual == expected);
    }
}

template<class Error>
void expect_rejection(Solver solve, const sirkit::WGraph& graph, int start, Distance inf) {
    std::vector<Distance> actual{123};
    bool rejected = false;
    try {
        (void)solve(graph, actual, start, inf);
    } catch (const Error&) {
        rejected = true;
    }
    assert(rejected);
#ifndef SKIP_RUNTIME_CHECKS
    assert(actual == std::vector<Distance>{123});
#endif
}

void sparse_reachability() {
    for (int size : {4'095, 4'096}) {
        sirkit::WGraph graph(size);
        graph[31] = {{12, 8}, {9, 2}, {6, 14}};
        graph[12] = {{23, -3}};
        graph[9] = {{12, -2}};
        graph[23] = {{7, 4}};
        graph[size - 1] = {{size - 1, -1}};
        std::vector<Distance> expected(size, sirkit::kInf);
        expected[31] = expected[12] = 0;
        expected[9] = 2;
        expected[6] = 14;
        expected[23] = -3;
        expected[7] = 1;
        check(graph, 31, sirkit::kInf, expected, false);

        // 12→23→12の負閉路は7へも届く。他の有限枝と未到達成分は保つ。
        graph[23].emplace_back(12, 2);
        expected[12] = expected[23] = expected[7] = -sirkit::kInf;
        check(graph, 31, sirkit::kInf, expected, true);
    }
}

void custom_inf() {
    constexpr Distance kCustomInf = 17;
    for (int size : {64, 65}) {
        sirkit::WGraph graph(size);
        graph[0] = {{1, 3}, {2, 16}, {3, -16}};
        graph[1] = {{1, -1}, {4, 0}};
        graph[2] = {{1, 0}};
        std::vector<Distance> expected(size, kCustomInf);
        expected[0] = 0;
        expected[1] = expected[4] = -kCustomInf;
        expected[2] = 16;
        expected[3] = -16;
        check(graph, 0, kCustomInf, expected, true);

        for (Distance weight : {-18, -17, 17, 18}) {
            graph[5] = {{6, weight}};
            for (const auto solve : kSolvers)
                expect_rejection<std::overflow_error>(solve, graph, 5, kCustomInf);
        }
        for (const auto solve : kSolvers)
            expect_rejection<std::invalid_argument>(solve, graph, 0, 0);
    }
}

void spfa_dispatch_limits() {
    constexpr auto kVertexLimit = static_cast<std::size_t>(std::numeric_limits<int>::max());
    assert(sirkit::detail::SpfaVertexCount(kVertexLimit) == std::numeric_limits<int>::max());
    bool rejected = false;
    try {
        (void)sirkit::detail::SpfaVertexCount(kVertexLimit + 1);
    } catch (const std::length_error&) {
        rejected = true;
    }
    assert(rejected);

    sirkit::WGraph graph(65);
    std::vector<Distance> expected(65, sirkit::kInf);
    expected[0] = 0;
    check(graph, 0, sirkit::kInf, expected, false);
    graph[0] = {{1, -17}};
    graph[1] = {{2, 7}};
    expected[1] = -17;
    expected[2] = -10;
    // 距離表の変換を伴う内部経路を、小さな入力で既定の有界経路と照合する。
    std::vector<Distance> distance{123};
    const bool negative = sirkit::detail::SpfaDefaultLarge<2'000'000'000>(graph, distance, 0);
    assert(!negative && distance == expected);
    check(graph, 0, sirkit::kInf, expected, false);
}

#ifndef SKIP_RUNTIME_CHECKS
void invalid_edges_and_vertices() {
    constexpr int kSize = 65;
    for (const auto solve : kSolvers) {
        const sirkit::WGraph empty(kSize);
        for (int start : {-1, kSize})
            expect_rejection<std::out_of_range>(solve, empty, start, sirkit::kInf);
        for (const sirkit::Edge invalid : {sirkit::Edge{-1, 0}, sirkit::Edge{kSize, 0},
                                          sirkit::Edge{5, -1'000'000'001},
                                          sirkit::Edge{5, 1'000'000'001}}) {
            auto graph = empty;
            graph[0] = {invalid};
            expect_rejection<std::out_of_range>(solve, graph, 0, sirkit::kInf);
            graph[0] = {{4, 0}, invalid};
            expect_rejection<std::out_of_range>(solve, graph, 0, sirkit::kInf);

            // 閉路の検出後に初めて参照する辺も検査する。
            graph[0] = {{1, 0}};
            graph[1] = {{0, -1}, invalid};
            expect_rejection<std::out_of_range>(solve, graph, 0, sirkit::kInf);
            graph[0] = {{0, -1}, {2, 0}};
            graph[2] = {invalid};
            expect_rejection<std::out_of_range>(solve, graph, 0, sirkit::kInf);
        }
    }
}
#endif

}  // namespace

int main() {
    sparse_reachability();
    custom_inf();
    spfa_dispatch_limits();
#ifndef SKIP_RUNTIME_CHECKS
    invalid_edges_and_vertices();
#endif
}
