// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Dijkstra.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <tuple>
#include <vector>

namespace {

using Distance = std::int64_t;
using Edge = std::tuple<int, int, Distance>;
constexpr Distance kDistanceLimit = 64;
constexpr Distance kUnreachable = std::numeric_limits<Distance>::max();
constexpr int kGraphSize = 4'097;

std::vector<Distance> reference_distances(int size, const std::vector<Edge>& edges) {
    std::vector<Distance> distances(size, kUnreachable);
    distances[0] = 0;
    for (int pass = 1; pass < size; ++pass) {
        bool changed = false;
        for (const auto& [from, to, weight] : edges) {
            if (distances[from] == kUnreachable || distances[from] + weight >= distances[to]) continue;
            distances[to] = distances[from] + weight;
            changed = true;
        }
        if (!changed) break;
    }
    return distances;
}

template <bool RestorePath, class Run>
void check_result(const std::vector<Edge>& edges, const std::vector<Distance>& reference,
                  int target, Run run) {
    const bool expected_overflow = std::ranges::any_of(reference, [](Distance distance) {
        return distance != kUnreachable && distance >= kDistanceLimit;
    });
    std::vector<Distance> distances{-1};
    std::vector<int> path{-1};
    bool thrown = false;
    try {
        run(distances, &path);
    } catch (const std::overflow_error&) {
        thrown = true;
    }
    assert(thrown == expected_overflow);
    if (thrown) {
#ifndef SKIP_RUNTIME_CHECKS
        // 検査有効時は、例外を確定するまで呼び出し側の値を書き換えない。
        assert(distances == std::vector<Distance>{-1});
#endif
        assert(path == std::vector<int>{-1});
        return;
    }

    assert(distances.size() == reference.size());
    for (std::size_t vertex = 0; vertex < distances.size(); ++vertex) {
        const Distance expected = reference[vertex] == kUnreachable ? kDistanceLimit : reference[vertex];
        assert(distances[vertex] == expected);
    }
    if constexpr (RestorePath) {
        if (reference[target] == kUnreachable) {
            assert(path.empty());
            return;
        }
        assert(!path.empty() && path.front() == 0 && path.back() == target);
        assert(path.size() <= distances.size());
        for (std::size_t i = 1; i < path.size(); ++i) {
            const auto from = path[i - 1], to = path[i];
            assert(std::ranges::any_of(edges, [&](const Edge& edge) {
                return std::get<0>(edge) == from && std::get<1>(edge) == to &&
                       reference[from] + std::get<2>(edge) == reference[to];
            }));
        }
    } else assert(path == std::vector<int>{-1});
}

template <bool RestorePath, class Graph>
void check_graph(const Graph& graph, const std::vector<Edge>& edges,
                 const std::vector<Distance>& reference, int target) {
    check_result<RestorePath>(edges, reference, target, [&](auto& distances, auto* path) {
        sirkit::detail::DijkstraDefaultLarge<RestorePath, kDistanceLimit>(
            graph, distances, 0, RestorePath ? target : -1, RestorePath ? path : nullptr);
    });
}

void check_case(const std::vector<Edge>& edges, int target) {
    const auto reference = reference_distances(kGraphSize, edges);
    sirkit::WGraph graph(kGraphSize);
    for (const auto& [from, to, weight] : edges) graph[from].emplace_back(to, weight);
    const sirkit::detail::DijkstraCsrGraph csr(kGraphSize, edges);
    check_graph<false>(graph, edges, reference, target);
    check_graph<true>(graph, edges, reference, target);
    check_graph<false>(csr, edges, reference, target);
    check_graph<true>(csr, edges, reference, target);
}

void check_radix_handoff() {
    std::vector<Edge> edges{{0, 1, 60}};
    // 一つの行で65件をキューに積み、残りの辺をradixへ引き継ぐ。
    for (int vertex = 2; vertex <= 70; ++vertex) {
        edges.emplace_back(1, vertex, 1);
        edges.emplace_back(vertex, 71, 0);
    }
    edges.emplace_back(1, 72, 4);
    edges.emplace_back(71, 72, 2);
    check_case(edges, 72);
    edges.pop_back();
    check_case(edges, 72);
}

} // namespace

int main() {
    // CSRの辺数だけを渡し、spanや辺配列を確保せず容量超過を確認する。
    bool rejected_capacity = false;
    try {
        sirkit::detail::DijkstraCsrGraph::check_capacity(0, std::numeric_limits<std::size_t>::max());
    } catch (const std::length_error&) {
        rejected_capacity = true;
    }
    assert(rejected_capacity);
    // 本番のモード切替境界は頂点配列を確保せずに検証する。
    constexpr auto limit = static_cast<std::size_t>((sirkit::kInf - 1) / 1'000'000'000);
    assert(!sirkit::detail::DijkstraMayOverflow(limit));
    assert(sirkit::detail::DijkstraMayOverflow(limit + 1));

    // 上限到達、上限超過、上限未満で閉じる一本道。
    check_case({{0, 1, 63}, {1, 2, 1}}, 2);
    check_case({{0, 1, 63}, {1, 2, 2}}, 2);
    check_case({{0, 1, 62}, {1, 2, 1}}, 2);
    check_case({{0, 1, 63}, {1, 0, 1}}, 1);
    // 始点の並行辺で初期一本道の検査を抜け、待ち行列が空の緩和を通る。
    check_case({{0, 1, 63}, {0, 1, 63}, {1, 2, 1}}, 2);
    check_case({{0, 1, 63}, {0, 1, 63}, {1, 0, 1}}, 1);

    // 越境した候補の行先が既に有限か、後から有限になるかを区別する。
    const std::vector<Edge> fork{{0, 1, 60}, {1, 2, 4}, {1, 0, 4}, {1, 3, 1}, {3, 2, 1}};
    check_case(fork, 2);
    auto without_shortcut = fork;
    without_shortcut.pop_back();
    check_case(without_shortcut, 2);
    check_case({{0, 1, 20}, {0, 2, 30}, {0, 3, 3}, {3, 4, 63}, {3, 1, 63}, {1, 4, 1}}, 4);
    check_radix_handoff();
}
