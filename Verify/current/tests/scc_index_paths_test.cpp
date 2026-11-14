// SPDX-License-Identifier: CC0-1.0
namespace sirkit {}
using namespace sirkit;
#ifndef SIRKIT_SCC_TEST_SOURCE
#define SIRKIT_SCC_TEST_SOURCE "../../../Graph/SCC.cpp"
#endif
#include SIRKIT_SCC_TEST_SOURCE
#include <algorithm>
#include <cinttypes>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <utility>
#include <vector>

namespace {
using Edges = std::vector<std::pair<int, int>>;
using Result = std::pair<int, std::vector<int>>;
std::uint64_t checks = 0, graphs = 0;
void check(bool ok) { ++checks; if (!ok) std::abort(); }

// low-linkを使わない2回のDFSによる独立参照。
Result reference(int n, const Edges& edges) {
    Graph forward(n), reverse(n);
    for (auto [u, v] : edges) { forward[u].push_back(v); reverse[v].push_back(u); }
    std::vector<unsigned char> seen(n);
    std::vector<int> finish;
    std::vector<std::pair<int, std::size_t>> frames;
    for (int root = 0; root < n; ++root) {
        if (seen[root]) continue;
        seen[root] = 1;
        frames.emplace_back(root, 0);
        while (!frames.empty()) {
            auto& [v, next] = frames.back();
            if (next == forward[v].size()) {
                finish.push_back(v); frames.pop_back();
            } else {
                const int u = forward[v][next++];
                if (!seen[u]) { seen[u] = 1; frames.emplace_back(u, 0); }
            }
        }
    }
    std::vector<int> ids(n, -1), pending;
    int count = 0;
    for (auto it = finish.rbegin(); it != finish.rend(); ++it) {
        if (ids[*it] != -1) continue;
        ids[*it] = count; pending.push_back(*it);
        while (!pending.empty()) {
            const int v = pending.back(); pending.pop_back();
            for (int u : reverse[v]) if (ids[u] == -1) {
                ids[u] = count; pending.push_back(u);
            }
        }
        ++count;
    }
    return {count, std::move(ids)};
}

Result verify(int n, const Edges& edges) {
    const auto narrow = ::sirkit::detail::directed_component_ids_index<std::uint32_t>(n, edges);
    const auto wide = ::sirkit::detail::directed_component_ids_index<std::size_t>(n, edges);
    const auto selected = ::sirkit::detail::directed_component_ids(n, edges);
    check(narrow == wide);
    check(narrow == selected);
    check(narrow.second.size() == static_cast<std::size_t>(n));
    const auto expected = reference(n, edges);
    check(narrow.first == expected.first);
    std::vector<int> expected_to_actual(expected.first, -1), actual_to_expected(expected.first, -1);
    for (int v = 0; v < n; ++v) {
        const int id = narrow.second[v], ref = expected.second[v];
        check(0 <= id && id < narrow.first);
        if (expected_to_actual[ref] == -1) expected_to_actual[ref] = id;
        if (actual_to_expected[id] == -1) actual_to_expected[id] = ref;
        check(expected_to_actual[ref] == id && actual_to_expected[id] == ref);
    }
    for (auto [u, v] : edges)
        check(narrow.second[u] == narrow.second[v] || narrow.second[u] < narrow.second[v]);
    if (n <= 12) {
        auto reach = std::vector(n, std::vector<bool>(n));
        for (int v = 0; v < n; ++v) reach[v][v] = true;
        for (auto [u, v] : edges) reach[u][v] = true;
        for (int k = 0; k < n; ++k)
            for (int u = 0; u < n; ++u)
                for (int v = 0; v < n; ++v)
                    reach[u][v] = reach[u][v] || (reach[u][k] && reach[k][v]);
        for (int u = 0; u < n; ++u)
            for (int v = 0; v < n; ++v)
                check((narrow.second[u] == narrow.second[v]) == (reach[u][v] && reach[v][u]));
    }
    ++graphs;
    return narrow;
}
}

int main() {
    // 全頂点対について辺の有無を列挙し、自己辺・無辺も含める。
    for (int n = 0; n <= 4; ++n) {
        const unsigned total = 1u << (n * n);
        for (unsigned mask = 0; mask < total; ++mask) {
            Edges edges;
            for (int u = 0; u < n; ++u)
                for (int v = 0; v < n; ++v)
                    if ((mask >> (u * n + v)) & 1u) edges.emplace_back(u, v);
            verify(n, edges);
        }
    }
    for (int n : {0, 1, 2, 3, 31, 32, 33, 1024}) {
        auto result = verify(n, {});
        check(result.first == n);
        for (int v = 0; v < n; ++v) check(result.second[v] == n - 1 - v);
    }
    // 未完了の兄弟、確定済み成分への辺、自己辺、多重辺を含む。
    verify(6, {{0,1},{1,2},{2,0},{0,3},{3,1},{4,5},{5,4},{5,2},{4,4},{4,5}});
    verify(6, {{0,1},{1,2},{2,1},{0,3},{3,2},{3,4},{4,5},{5,5},{5,5}});
    std::mt19937_64 random(20261103);
    for (int n : {1, 2, 3, 5, 12, 19, 257, 4096}) {
        for (int trial = 0; trial < 150; ++trial) {
            Edges edges;
            for (int i = 0; i < 4 * n; ++i)
                edges.emplace_back(static_cast<int>(random() % n), static_cast<int>(random() % n));
            const auto first = verify(n, edges);
            std::shuffle(edges.begin(), edges.end(), random);
            const auto shuffled = verify(n, edges);
            check(first.first == shuffled.first);
            for (int i = 0; i < n; ++i)
                edges.emplace_back(i, i);
            const auto self = verify(n, edges);
            check(shuffled == self);
        }
    }
    // 関数グラフの流入木・複数閉路と、m=nでも出次数が0/2になる近傍を独立照合。
    for (int n : {1, 2, 3, 5, 12, 19, 257, 4096}) {
        for (int trial = 0; trial < 20; ++trial) {
            Edges functional;
            for (int v = 0; v < n; ++v)
                functional.emplace_back(v, static_cast<int>(random() % n));
            verify(n, functional);
            std::shuffle(functional.begin(), functional.end(), random);
            verify(n, functional);
            auto missing = functional;
            missing.pop_back();
            verify(n, missing);
            if (n > 1) {
                auto near = functional;
                near[0].first = near[1].first;
                verify(n, near);
            }
        }
    }
    const int deep = 300000;
    for (int shape = 0; shape < 3; ++shape) {
        Edges edges;
        for (int v = 1; v < deep; ++v)
            edges.emplace_back(shape == 1 ? v : v - 1, shape == 1 ? v - 1 : v);
        if (shape == 2) edges.emplace_back(deep - 1, 0);
        const auto result = verify(deep, edges);
        check(result.first == (shape == 2 ? 1 : deep));
        for (int v = 0; v < deep; ++v)
            check(result.second[v] == (shape == 2 ? 0 : shape == 1 ? deep - 1 - v : v));
    }
    // 最大値そのものもCSR終端値として表せる（巨大入力の実確保はしない）。
    constexpr auto last = std::numeric_limits<std::uint32_t>::max();
    static_assert(static_cast<std::uint64_t>(last) + 1 > last);
    std::printf("SCC index paths graphs=%" PRIu64 " checks=%" PRIu64 "\n", graphs, checks);
}
