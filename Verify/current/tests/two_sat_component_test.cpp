// SPDX-License-Identifier: CC0-1.0
#ifndef SIRKIT_TWO_SAT_TEST_SOURCE
#define SIRKIT_TWO_SAT_TEST_SOURCE "../../../Graph/Two_SAT.cpp"
#endif
#ifndef SIRKIT_SCC_TEST_SOURCE
#define SIRKIT_SCC_TEST_SOURCE "../../../Graph/SCC.cpp"
#endif
#ifdef SIRKIT_TWO_SAT_TEST_SCC_FIRST
#include SIRKIT_SCC_TEST_SOURCE
#include SIRKIT_TWO_SAT_TEST_SOURCE
#else
#include SIRKIT_TWO_SAT_TEST_SOURCE
#include SIRKIT_SCC_TEST_SOURCE
#endif
#include SIRKIT_TWO_SAT_TEST_SOURCE
#include SIRKIT_SCC_TEST_SOURCE
#include <algorithm>
#include <cinttypes>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <random>
#include <tuple>
#include <utility>
#include <vector>

namespace {
using Edges = std::vector<std::pair<int, int>>;
using Clause = std::tuple<int, bool, int, bool>;
std::uint64_t checks = 0, graphs = 0;
void check(bool ok) { ++checks; if (!ok) std::abort(); }

bool verify(int n, const Edges& edges) {
    check(n >= 0 && n % 2 == 0);
    const auto reference = reinsirk::detail::directed_component_ids(n, edges);
    bool contradiction = false;
    for (int i = 0; i < n; i += 2)
        contradiction = contradiction || reference.second[i] == reference.second[i + 1];
    const auto narrow = reinsirk::detail::two_sat_component_ids_index<std::uint32_t>(n, edges);
    const auto wide = reinsirk::detail::two_sat_component_ids_index<std::size_t>(n, edges);
    const auto selected = reinsirk::detail::two_sat_component_ids(n, edges);
    for (const auto* result : {&narrow, &wide, &selected}) {
        if (contradiction) {
            check(result->first < 0);
            check(result->second.empty());
        } else {
            check(*result == reference);
            check(result->second.size() == static_cast<std::size_t>(n));
        }
    }
    ++graphs;
    return !contradiction;
}
void formula(int n, const std::vector<Clause>& clauses) {
    Edges edges;
    two_sat constraints(n);
    for (auto [i, a, j, b] : clauses) {
        constraints.add_clause(i, a, j, b);
        edges.emplace_back(2 * i + !a, 2 * j + b);
        edges.emplace_back(2 * j + !b, 2 * i + a);
    }
    const bool possible = verify(2 * n, edges);
    check(constraints.satisfiable() == possible);
    const auto answer = constraints.answer();
    check(answer.size() == static_cast<std::size_t>(n));
    if (possible)
        for (auto [i, a, j, b] : clauses)
            check((answer[i] == a) || (answer[j] == b));
}
}

int main() {
    // 偶数頂点の全小規模グラフ。節へ変換できない辺集合もSCCの矛盾判定そのものを検査する。
    for (int n : {0, 2, 4}) {
        for (unsigned mask = 0; mask < (1u << (n * n)); ++mask) {
            Edges edges;
            for (int u = 0; u < n; ++u)
                for (int v = 0; v < n; ++v)
                    if ((mask >> (u * n + v)) & 1u) edges.emplace_back(u, v);
            verify(n, edges);
        }
    }
    for (int n : {0, 2, 4, 8, 64, 258}) {
        check(verify(n, {}));
        Edges self;
        for (int v = 0; v < n; ++v) self.emplace_back(v, v);
        check(verify(n, self));
    }
    // 相補頂点は別々の非singleton SCCに入るためSAT。
    check(verify(4, {{0,2},{2,0},{1,3},{3,1}}));
    check(verify(6, {{0,2},{2,4},{4,0},{1,3},{3,5},{5,1}}));
    // 相補対の2個目がroot自身となり、popの最後で初めて矛盾を検出する。
    check(!verify(8, {{0,2},{2,4},{4,6},{6,1},{1,0}}));
    // 先行singletonによりraw countが0ではない状態で同じ条件を検査する。
    check(!verify(6, {{2,4},{4,3},{3,2}}));
    // activeな親の退避idsがraw countと同じでも、確定済みと誤認しない。
    check(verify(4, {{0,1},{1,3},{3,1}}));
    check(verify(6, {{0,0},{1,1},{2,3},{3,5},{5,3}}));
    check(!verify(4, {{0,1},{0,1},{1,0},{1,0},{2,2},{3,3}}));

    std::mt19937_64 random(20261104);
    for (int n : {1, 2, 3, 8, 32, 257, 4096}) {
        const int rounds = n == 4096 ? 20 : 80;
        for (int trial = 0; trial < rounds; ++trial) {
            std::vector<bool> planted(n);
            for (int i = 0; i < n; ++i) planted[i] = (random() & 1) != 0;
            std::vector<Clause> clauses;
            for (int m = 0; m < 4 * n; ++m) {
                const int i = static_cast<int>(random() % n), j = static_cast<int>(random() % n);
                bool a = (random() & 1) != 0, b = (random() & 1) != 0;
                if (trial % 2 == 0 && planted[i] != a && planted[j] != b) b = !b;
                clauses.emplace_back(i, a, j, b);
            }
            formula(n, clauses);
            std::reverse(clauses.begin(), clauses.end());
            formula(n, clauses);
            clauses.emplace_back(0, true, 0, true);
            clauses.emplace_back(0, false, 0, false);
            formula(n, clauses);
        }
    }
    const int deep = 300000;
    for (int kind = 0; kind < 3; ++kind) {
        Edges edges;
        for (int v = 1; v < deep; ++v)
            edges.emplace_back(kind == 1 ? v : v - 1, kind == 1 ? v - 1 : v);
        if (kind == 2) edges.emplace_back(deep - 1, 0);
        check(verify(deep, edges) == (kind != 2));
    }
    // 相補SCCがそれぞれ深い閉路を持つSATを検査する。
    Edges separate;
    for (int i = 0; i < deep / 2; ++i) {
        const int next = (i + 1) % (deep / 2);
        separate.emplace_back(2 * i, 2 * next);
        separate.emplace_back(2 * next + 1, 2 * i + 1);
    }
    check(verify(deep, separate));
    std::printf("TwoSAT component graphs=%" PRIu64 " checks=%" PRIu64 "\n", graphs, checks);
}
