#define SKIP_RUNTIME_CHECKS
// Profile: Fast/GCC; online judgement is recorded separately.
// Verify: https://atcoder.jp/contests/abc137/tasks/abc137_e

// Library: Graph/Bellman-Ford.cpp
// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Bellman-Ford.cpp
// 参考: https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/bellman-ford
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/negative_shortest_paths.md
//
// API一覧
// [共通型・定数]
// Edge(to,w): 行先toとint64_tの重みwを持つ有向辺。
// WGraph: 出発点を隣接リストの添字とする辺列。kInf=std::int64_t{1}<<60。
//
// [Bellman-Ford]
// BellmanFord(graph, distances, start_index): 始点からの最短距離をdistancesに格納し、到達可能な負閉路があればtrueを返す。
// SKIP_RUNTIME_CHECKSを定義すると入力検査を省く。
// 検査は始点と処理中に参照する辺のみ。未到達頂点の出辺は走査しない。
// 始点から到達できる負閉路があればtrue。未到達はkInf、負閉路の影響下は-kInf。
// distancesは成功時に全上書き。有限距離が(-kInf, kInf)の範囲外ならoverflow_error。
// 重みは[-1e9, 1e9]。O(VE+V+E)時間・O(V)追加領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_INF_DEFINED
#define SIRKIT_INF_DEFINED
inline constexpr std::int64_t kInf = std::int64_t{1} << 60;
#endif

#ifndef SIRKIT_GRAPH_TYPES_DEFINED
#define SIRKIT_GRAPH_TYPES_DEFINED
struct Edge
{
    int to;
    std::int64_t w;
    Edge(int destination, std::int64_t weight) : to(destination), w(weight) {}
};
using WGraph = std::vector<std::vector<Edge>>;
#endif

// 小規模・少数始点は隣接リストを走査し、多数パスでは更新頂点に処理を絞る。
#ifndef SIRKIT_BELLMAN_FORD_DEFINED
#define SIRKIT_BELLMAN_FORD_DEFINED
namespace /*SIRKIT_NS_REWRITE_V1:D:cmVpbnNpcms6OmRldGFpbA==*/detail/*SIRKIT_NS_REWRITE_END_V1*/ {
// 遅い反復だけを到達部分の強連結成分ごとに解く。追加領域O(V)。
[[gnu::noinline]]
inline void BellmanFordComponents(const WGraph& graph, std::span<const int> sources,
    std::vector<std::int64_t>& distance, std::int64_t unreachable, std::int64_t lower_bound,
    std::vector<int>& negative
) {
    struct Frame { int vertex; std::size_t edge; };
    std::vector<int> order(graph.size(), -1), low(graph.size()), component(graph.size(), -1);
    std::vector<int> active, members;
    std::vector<std::int64_t> tree_distance(graph.size());
    // stateの下位bitは現在のDFS祖先、次のbitは負閉路の証拠。
    std::vector<std::uint8_t> state(graph.size(), 0), negative_component;
    std::vector<std::size_t> boundary{0};
    std::vector<Frame> dfs;
    int clock = 0;
    const auto discover = [&](int vertex, std::int64_t tree_value) {
        tree_distance[vertex] = tree_value;
        state[vertex] = 1;
        order[vertex] = low[vertex] = clock++;
        active.push_back(vertex);
        dfs.push_back({vertex, 0});
    };
    for (const int root : sources) {
        if (distance[root] == unreachable || order[root] >= 0) continue;
        discover(root, 0);
        while (!dfs.empty()) {
            auto& frame = dfs.back();
            const int from = frame.vertex;
            if (frame.edge != graph[from].size()) {
                const auto& edge = graph[from][frame.edge++];
                if (order[edge.to] < 0) discover(edge.to, tree_distance[from] + edge.w);
                else if (component[edge.to] < 0) {
                    low[from] = std::min(low[from], order[edge.to]);
                    // DFS木上の祖先へ戻る辺で、閉路の重みが負と確定する。
                    if ((state[edge.to] & 1) && tree_distance[from] + edge.w < tree_distance[edge.to])
                        state[from] |= 2;
                }
            } else {
                if (low[from] == order[from]) {
                    const int id = static_cast<int>(boundary.size() - 1);
                    bool found_negative = false;
                    for (;;) {
                        const int vertex = active.back(); active.pop_back();
                        component[vertex] = id;
                        found_negative |= (state[vertex] & 2) != 0;
                        members.push_back(vertex);
                        if (vertex == from) break;
                    }
                    boundary.push_back(members.size());
                    negative_component.push_back(found_negative);
                }
                state[from] &= 2;
                dfs.pop_back();
                if (!dfs.empty()) low[dfs.back().vertex] = std::min(low[dfs.back().vertex], low[from]);
            }
        }
    }
    const auto mark = [&](int vertex) {
        if (distance[vertex] != lower_bound) {
            negative.push_back(vertex);
            distance[vertex] = lower_bound;
        }
    };
    for (std::size_t block = boundary.size() - 1; block != 0; --block) {
        const int id = static_cast<int>(block - 1);
        const auto first = boundary[block - 1], last = boundary[block], size = last - first;
        bool affected = negative_component[id] != 0, ring = true;
        for (auto position = first; position != last; ++position) {
            const int from = members[position];
            affected |= distance[from] == lower_bound;
            if (graph[from].size() != 1 || component[graph[from].front().to] != id) ring = false;
        }
        if (!affected && ring) {
            int from = members[first];
            for (std::size_t step = 0; step < 2 * size; ++step) {
                const auto& edge = graph[from].front();
                if (distance[from] != unreachable)
                    distance[edge.to] = std::min(distance[edge.to], distance[from] + edge.w);
                from = edge.to;
            }
        } else if (!affected && size > 1) {
            std::vector<int> next;
            active.clear();
            for (auto position = first; position != last; ++position) {
                const int from = members[position];
                order[from] = distance[from] != unreachable;
                if (order[from]) active.push_back(from);
            }
            bool converged = false;
            for (std::size_t pass = 1; pass < std::min(size, std::size_t{8}); ++pass) {
                for (const int from : active) {
                    order[from] = 0;
                    for (const auto& edge : graph[from]) {
                        if (component[edge.to] != id) continue;
                        const auto candidate = distance[from] + edge.w;
                        if (candidate < distance[edge.to]) {
                            if (candidate <= lower_bound) { affected = true; break; }
                            distance[edge.to] = candidate;
                            if (!order[edge.to]) {
                                next.push_back(edge.to);
                                order[edge.to] = 1;
                            }
                        }
                    }
                    if (affected) break;
                }
                if (affected) break;
                if (next.empty()) { converged = true; break; }
                active.swap(next);
                next.clear();
            }
            if (!affected && !converged && size > 8) {
                // 少数回で収束する成分の順序は保ち、残りだけ反転する。
                std::reverse(active.begin(), active.end());
                // Tarjanで使い終えた配列を前駆頂点へ転用する。
                for (auto position = first; position != last; ++position) low[members[position]] = -1;
                std::size_t cycle_budget = size;
                for (std::size_t pass = 8; pass < size; ++pass) {
                    for (const int from : active) {
                        order[from] = 0;
                        for (const auto& edge : graph[from]) {
                            if (component[edge.to] != id) continue;
                            const auto candidate = distance[from] + edge.w;
                            if (candidate < distance[edge.to]) {
                                if (candidate <= lower_bound) { affected = true; break; }
                                distance[edge.to] = candidate;
                                low[edge.to] = from;
                                if (cycle_budget != 0) --cycle_budget;
                                if (!order[edge.to]) {
                                    next.push_back(edge.to);
                                    order[edge.to] = 1;
                                }
                            }
                        }
                        if (affected) break;
                    }
                    if (affected) break;
                    if (next.empty()) { converged = true; break; }
                    // 厳密な緩和で得た前駆閉路は負。検出費用は緩和回数に応じて抑える。
                    if (cycle_budget == 0) {
                        for (auto position = first; position != last; ++position) state[members[position]] = 0;
                        for (auto position = first; position != last; ++position) {
                            const int root = members[position];
                            int at = root;
                            while (low[at] >= 0 && state[at] == 0) { state[at] = 1; at = low[at]; }
                            if (low[at] >= 0 && state[at] == 1) { affected = true; break; }
                            at = root;
                            while (low[at] >= 0 && state[at] == 1) { state[at] = 2; at = low[at]; }
                        }
                        cycle_budget = size;
                        if (affected) break;
                    }
                    active.swap(next);
                    next.clear();
                }
            }
            if (!affected && !converged) {
                for (auto position = first; position != last; ++position) {
                    const int from = members[position];
                    if (distance[from] == unreachable) continue;
                    for (const auto& edge : graph[from])
                        if (component[edge.to] == id && distance[from] + edge.w < distance[edge.to]) affected = true;
                }
            }
        }
        if (affected)
            for (auto position = first; position != last; ++position) mark(members[position]);
        for (auto position = first; position != last; ++position) {
            const int from = members[position];
            if (distance[from] == unreachable) continue;
            for (const auto& edge : graph[from]) {
                if (component[edge.to] == id) continue;
                if (affected) mark(edge.to);
                else {
                    const auto candidate = distance[from] + edge.w;
                    if (candidate < distance[edge.to]) {
                        if (candidate <= lower_bound) mark(edge.to);
                        else distance[edge.to] = candidate;
                    }
                }
            }
        }
    }
}


// 反復1回の仕事量が大きい場合だけ成分分解を再試行する。
[[gnu::noinline, gnu::cold]]
inline bool BellmanFordNeedsComponents(const WGraph& graph, std::span<const int> frontier) {
    if (frontier.size() >= std::max(std::size_t{64}, graph.size() / 16)) return true;
    auto budget = graph.size();
    for (const int from : frontier) {
        const auto degree = graph[from].size();
        if (degree >= budget) return true;
        budget -= degree;
    }
    return false;
}

// 3パス目以降は距離が更新された始点だけを次のパスへ送る。
// 1パスで各頂点を高々1回処理するので、時間上限O(VE)を保つ。
// 戻り値は0: 未収束、1: 収束、2: 成分処理で負影響も伝播済み。
[[gnu::noinline]]
inline int BellmanFordContinue(const WGraph& graph, std::span<const int> sources,
    std::vector<std::int64_t>& distance, std::int64_t unreachable, std::int64_t lower_bound,
    std::vector<int>& negative
) {
    const int n = static_cast<int>(graph.size());
    // 単純路の非終端頂点は出辺あり頂点に限られ、辺数はその個数以下。
    const int round_limit = sources.size() < graph.size() ? static_cast<int>(sources.size()) + 1 : n;
    std::vector<int> frontier, next;
    std::vector<std::uint8_t> scheduled(graph.size(), 0);
    for (const int from : sources) if (distance[from] != unreachable) {
        frontier.push_back(from);
        scheduled[from] = 1;
    }
    const auto process = [&](int from, auto validate) {
        for (const auto& edge : graph[from]) {
            (void)validate;
            const auto candidate = distance[from] + edge.w;
            if (candidate >= distance[edge.to]) continue;
            if (candidate <= lower_bound) [[unlikely]] {
                if (distance[edge.to] != lower_bound) {
                    negative.push_back(edge.to);
                    distance[edge.to] = lower_bound;
                }
            } else distance[edge.to] = candidate;
            if (!scheduled[edge.to] && !graph[edge.to].empty()) {
                next.push_back(edge.to);
                scheduled[edge.to] = 1;
            }
        }
    };
    // 少数回で収束する場合と更新頂点が少ない場合は、通常の反復を保つ。
    constexpr int component_pass = 16;
    std::size_t previous_frontier = 0;
    for (int pass = 3; pass < std::min(round_limit, component_pass); ++pass) {
        previous_frontier = frontier.size();
        for (const int from : frontier) {
            scheduled[from] = 0;
                process(from, std::false_type{});
        }
        if (next.empty()) return true;
        frontier.swap(next);
        next.clear();
    }
    if (round_limit > component_pass && frontier.size() >= previous_frontier &&
        frontier.size() >= std::max(std::size_t{64}, graph.size() / 16)) {
        BellmanFordComponents(graph, sources, distance, unreachable, lower_bound, negative
        );
        return 2;
    }
    // 64パスごとに両端の更新頂点数から仕事量を見積もり、長い反復を分解する。
    std::uint64_t component_budget = static_cast<std::uint64_t>(n) * 16;
    int pass = component_pass;
    while (pass < round_limit) {
        const auto first_count = frontier.size();
        const int first_pass = pass;
        const int end_pass = pass + std::min(64 - (pass & 63), round_limit - pass);
        for (; pass < end_pass; ++pass) {
            for (const int from : frontier) {
                scheduled[from] = 0;
                    process(from, std::false_type{});
            }
            if (next.empty()) return true;
            frontier.swap(next);
            next.clear();
        }
        if (pass == round_limit) break;
        const auto work = static_cast<std::uint64_t>(std::min(first_count,frontier.size())) * (pass-first_pass);
        if (work >= component_budget ||
            (pass == 64 && BellmanFordNeedsComponents(graph, frontier))) [[unlikely]] {
            BellmanFordComponents(graph,sources,distance,unreachable,lower_bound,negative
            );
            return 2;
        }
        component_budget -= work;
    }
    return false;
}

template <bool UseActive, bool Bounded = false, bool Warm = false, class... Initial>
[[gnu::noinline]]
inline bool BellmanFordImpl(const WGraph &graph, std::vector<std::int64_t> &distances,
                        int start_index, Initial... initial_state)
{
    const auto initial = std::tuple{initial_state...};
    auto* initial_distance = [&]() {
        if constexpr (Warm) return std::get<0>(initial);
        else return static_cast<std::vector<std::int64_t>*>(nullptr);
    }();
    constexpr std::int64_t max_cost = 1'000'000'000;
    if constexpr (!Warm) {
        (void)initial_distance;
    }
    if (graph.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("BellmanFord: too many vertices");
    const int n = static_cast<int>(graph.size());
    if (graph[start_index].empty()) {
        distances.assign(graph.size(), kInf);
        distances[start_index] = 0;
        return false;
    }
    std::vector<int> large_sources;
    alignas(64)
    std::array<int, UseActive ? 0 : 64> small_sources;
    std::size_t source_count = 0;
    if constexpr (UseActive) large_sources.reserve(std::min(graph.size(), std::size_t{64}));
    for (int from = 0; from < n; ++from)
        if (!graph[from].empty()) {
            if constexpr (UseActive) {
                // 多数の始点がある場合だけ、一度の拡張で再確保を抑える。
                if (large_sources.size() == large_sources.capacity()) large_sources.reserve(graph.size());
                large_sources.push_back(from);
            } else small_sources[source_count++] = from;
        }
    const std::span<const int> sources = [&]() -> std::span<const int> {
        if constexpr (UseActive) return large_sources;
        else return {small_sources.data(), source_count};
    }();

    // 単純路は(n-1)辺以下。候補も[-(n+1), n+2]*max_cost内なので加算は安全。
    const auto count = static_cast<std::int64_t>(n);
    constexpr bool direct_inf = Bounded || !UseActive;
    const std::int64_t unreachable = direct_inf ? kInf : (count + 1) * max_cost;
    const std::int64_t lower_bound = -count * max_cost;
    std::vector<std::int64_t> local_distance;
    alignas(64)
    std::array<std::int64_t, UseActive ? 0 : 64> small_distance;
    auto& distance = [&]() -> auto& {
        if constexpr (!UseActive) return small_distance;
        else return distances;
    }();
    if constexpr (UseActive) {
        if constexpr (Warm) {
            if (&distance != initial_distance) distance.swap(*initial_distance);
        } else distance.assign(graph.size(), unreachable);
    }
    else std::fill_n(distance.begin(), n, unreachable);
    std::vector<int> queue;
    distance[start_index] = 0;

    // 単純路では届かない下限を負影響の印にも使い、専用配列を省く。
    const auto mark_negative = [&](int vertex)
    {
        if (distance[vertex] == lower_bound) return;
        queue.push_back(vertex);
        distance[vertex] = lower_bound;
    };
    const auto relax_edges = [&](int from, auto validate, bool &changed)
    {
        for (const auto &edge : graph[from])
        {
            (void)validate;
            std::int64_t candidate = distance[from] + edge.w;
            if (candidate >= distance[edge.to])
                continue;
            // 単純路では届かない下限に達した場合だけ、負閉路の種を記録する。
            if (candidate <= lower_bound) [[unlikely]] {
                mark_negative(edge.to);
            } else {
                distance[edge.to] = candidate;
            }
            changed = true;
        }
    };
    const bool use_active = UseActive && sources.size() >= 64;
    // 初期パス数も、出辺あり頂点から得た上限で制限する。
    const int pass_limit = use_active ? std::min(n, 3) :
        (sources.size() < graph.size() ? static_cast<int>(sources.size()) + 1 : n);
    int converged = 0;
    for (int pass = 1; pass < pass_limit; ++pass)
    {
        bool changed = false;
        for (const int from : sources)
        {
            if (distance[from] == unreachable)
                continue;
                relax_edges(from, std::false_type{}, changed);
        }
        if (!changed)
        {
            converged = true;
            break;
        }
    }

    // 出辺を持つ頂点が始点だけなら、閉路は自己辺に限られる。
    if (sources.size() == 1 && n > 1) {
        if (distance[start_index] < 0) mark_negative(start_index);
        converged = true;
    }

    if constexpr (UseActive) {
        if (use_active && !converged && n > 3)
            converged = BellmanFordContinue(graph, sources, distance, unreachable, lower_bound, queue
            );
    }

    // さらに短くできる頂点を、負閉路の影響の種として記録する。
    if (!converged)
    {
        for (const int from : sources)
        {
            if (distance[from] == unreachable)
                continue;
            for (const auto &edge : graph[from])
            {
                if (distance[from] + edge.w < distance[edge.to])
                    mark_negative(edge.to);
            }
        }
    }
    // 成分処理が完了した場合は負影響も伝播済み。それ以外だけ辺をたどる。
    if (converged != 2)
        for (std::size_t head = 0; head < queue.size(); ++head)
            for (const auto &edge : graph[queue[head]])
                mark_negative(edge.to);

    if constexpr (direct_inf) {
        // n*最大辺重み<kInfなら有限距離はkInf未満。未到達の変換と範囲走査は不要。
        for (const int vertex : queue) distance[vertex] = -kInf;
    } else {
    for (int vertex = 0; vertex < n; ++vertex)
    {
        if (distance[vertex] == lower_bound)
        {
            distance[vertex] = -kInf;
        }
        else if (distance[vertex] == unreachable)
        {
            distance[vertex] = kInf;
        }
        else if (distance[vertex] <= -kInf || distance[vertex] >= kInf)
        {
            throw std::overflow_error("BellmanFord: finite distance collides with INF/-INF");
        }
    }
    }
    if constexpr (!UseActive) distances.assign(distance.begin(), distance.begin() + n);
    return !queue.empty();
}
// 到達部分が小さければ、全頂点の出辺有無を調べずに計算する。
// 試行は64頂点・128辺まで。上限を超えた場合は通常処理に戻る。
[[gnu::noinline]]
inline int BellmanFordSparse(const WGraph& graph,
    std::vector<std::int64_t>& distances, int start_index) {
    constexpr std::size_t vertex_limit = 64, edge_limit = 128;
    struct CompactEdge { int from, to; std::int64_t w; };
    std::array<int, vertex_limit> vertices;
    std::array<CompactEdge, edge_limit> edges;
    std::size_t vertex_count = 1, edge_count = 0;
    vertices[0] = start_index;
    for (std::size_t head = 0; head < vertex_count; ++head) {
        for (const auto& edge : graph[vertices[head]]) {
            if (edge_count == edge_limit) return 0;
            std::size_t target = 0;
            while (target < vertex_count && vertices[target] != edge.to) ++target;
            if (target == vertex_count) {
                if (vertex_count == vertex_limit) return 0;
                vertices[vertex_count++] = edge.to;
            }
            edges[edge_count++] = {static_cast<int>(head), static_cast<int>(target), edge.w};
        }
    }
    std::array<std::int64_t, vertex_limit> distance;
    std::fill_n(distance.begin(), vertex_count, kInf);
    distance[0] = 0;
    // 64パス・128辺・|w|<=1e9なので、負閉路があっても加算はint64_t内。
    for (std::size_t pass = 1; pass < vertex_count; ++pass) {
        bool changed = false;
        for (std::size_t i = 0; i < edge_count; ++i) {
            const auto& edge = edges[i];
            if (distance[edge.from] == kInf) continue;
            const auto candidate = distance[edge.from] + edge.w;
            if (candidate < distance[edge.to]) {
                distance[edge.to] = candidate;
                changed = true;
            }
        }
        if (!changed) break;
    }
    std::uint64_t negative = 0;
    for (std::size_t i = 0; i < edge_count; ++i) {
        const auto& edge = edges[i];
        if (distance[edge.from] != kInf && distance[edge.from] + edge.w < distance[edge.to])
            negative |= std::uint64_t{1} << edge.to;
    }
    if (negative != 0) {
        for (std::size_t pass = 1; pass < vertex_count; ++pass) {
            const auto previous = negative;
            for (std::size_t i = 0; i < edge_count; ++i) {
                const auto& edge = edges[i];
                if ((negative >> edge.from) & 1) negative |= std::uint64_t{1} << edge.to;
            }
            if (negative == previous) break;
        }
        for (std::size_t i = 0; i < vertex_count; ++i)
            if ((negative >> i) & 1) distance[i] = -kInf;
    }
    // 到達部分の検査・計算が全て成功してから出力を更新する。
    distances.assign(graph.size(), kInf);
    for (std::size_t i = 0; i < vertex_count; ++i) distances[vertices[i]] = distance[i];
    return negative == 0 ? 1 : 2;
}

// 距離の更新が番号の増加方向だけなら、1回の緩和で距離が確定する。
// 改善する後退辺で止め、初回緩和の距離を通常処理へ渡す。
// 直前まで一本道なら祖先への改善辺は負閉路。影響を伝播して終える。
// 戻り値は確定時-1、負影響伝播完了時-2、それ以外は通常処理へ渡す停止頂点。
[[gnu::noinline]]
inline int BellmanFordForward(const WGraph& graph,
    std::vector<std::int64_t>& distance, int start_index) {
    distance.assign(graph.size(), kInf);
    distance[start_index] = 0;
    const int n = static_cast<int>(graph.size());
    bool unique_prefix = true;
    for (int from = start_index; from < n; ++from) {
        const auto from_distance = distance[from];
        if (from_distance == kInf) continue;
        for (const auto& edge : graph[from]) {
            const auto candidate = from_distance + edge.w;
            if (candidate >= distance[edge.to]) continue;
            if (edge.to <= from) {
                if (!unique_prefix || distance[edge.to] == kInf) return from;
                // 処理済みの到達頂点は一本道。後退先からfromへの経路が存在する。
                // candidate<distance[to]は、この閉路の重みが負であることを表す。
                std::vector<int> negative;
                negative.push_back(edge.to);
                distance[edge.to] = -kInf;
                for (std::size_t head = 0; head < negative.size(); ++head) {
                    for (const auto& next : graph[negative[head]]) {
                        if (distance[next.to] != -kInf) {
                            negative.push_back(next.to);
                            distance[next.to] = -kInf;
                        }
                    }
                }
                return -2;
            }
            distance[edge.to] = candidate;
        }
        // 現在の頂点で初めて分岐した場合は、この行の祖先照合まで一本道の証明が使える。
        if (graph[from].size() > 1) unique_prefix = false;
    }
    return -1;
}

} // namespace reinsirk::detail

inline bool BellmanFord(const WGraph& graph, std::vector<std::int64_t>& distances, int start_index) {
    if (graph.size() <= 64) return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::BellmanFordImpl<false>(graph, distances, start_index);
    if (graph.size() >= 4096) {
        if (graph.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
            throw std::length_error("BellmanFord: too many vertices");
        const int sparse = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::BellmanFordSparse(graph, distances, start_index);
        if (sparse != 0) return sparse == 2;
    }
    if (graph.size() <= static_cast<std::size_t>((kInf - 1) / 1'000'000'000)
        && graph[start_index].size() == 1 && graph[start_index].front().to > start_index) {
        auto& initial = distances;
        const int stopped = /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::BellmanFordForward(graph, initial, start_index);
        if (stopped < 0) {
            return stopped == -2;
        }
        return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::BellmanFordImpl<true, true, true>(graph, distances, start_index, &initial
        );
    }
    if (graph.size() <= static_cast<std::size_t>((kInf - 1) / 1'000'000'000))
        return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::BellmanFordImpl<true, true>(graph, distances, start_index);
    return /*SIRKIT_NS_REWRITE_V1:R:cmVpbnNpcms6OmRldGFpbA==*/::sirkit::detail/*SIRKIT_NS_REWRITE_END_V1*/::BellmanFordImpl<true, false>(graph, distances, start_index);
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1


namespace sirkit {}
using namespace sirkit;

// SPDX-License-Identifier: CC0-1.0
// Verify I/O: Verify/current/common/fast_io.hpp
// SPDX-License-Identifier: CC0-1.0
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#ifndef SIRKIT_VERIFY_FAST_IO_DEFINED
#define SIRKIT_VERIFY_FAST_IO_DEFINED

// VerifyInput(file=stdin): 非所有のFILEから読む。別の入力方式と混用しない。
// >> 整数: 64bit以下の十進整数を読む。検査省略時は値域内の正しい入力を前提とする。
// >> char/string: 空白を飛ばし、1文字／空白までの文字列を読む。
// >> 浮動小数点: 十進文字列を変換する。通常のCロケールで使う。
// bool / !: 直前までの読み取りが成功しているか／失敗したかを返す。
class VerifyInput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<unsigned char, buffer_size + 1> buffer_;
    std::size_t position_ = 0, size_ = 0;
    bool good_ = true;

    int peek() {
        if (position_ == size_) {
            size_ = std::fread(buffer_.data(), 1, buffer_size, file_);
            buffer_[size_] = 0; // 補充後の末尾は常に数字でない番兵。
            position_ = 0;
            if (size_ == 0) return EOF;
        }
        return buffer_[position_];
    }

    int skip_space() {
        int c;
        while ((c = peek()) != EOF && c <= ' ') ++position_;
        if (c == EOF) good_ = false;
        return c;
    }

public:
    explicit VerifyInput(std::FILE* file = stdin) : file_(file) {}
    VerifyInput(const VerifyInput&) = delete;
    VerifyInput& operator=(const VerifyInput&) = delete;

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyInput& operator>>(T& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        bool negative = false;
        if (c == '-' || c == '+') {
            negative = c == '-';
            ++position_;
            c = peek();
        }
        if (c == EOF) {
            good_ = false;
            return *this;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (c < '0' || c > '9') {
            good_ = false;
            return *this;
        }
        // 先頭の0を除けば、20桁以下の末尾直前の値はuint64_tに収まる。
        while (c == '0') {
            ++position_;
            c = peek();
        }
        std::uint64_t previous = 0;
        unsigned last_digit = 0;
        std::size_t digits = 0;
#endif
        std::uint64_t value = 0;
#ifdef SKIP_RUNTIME_CHECKS
        // バッファ末尾の番兵で内側の範囲判定を省き、数字が境界をまたぐときだけ補充する。
        for (;;) {
            const unsigned char* first = buffer_.data() + position_;
            const unsigned char* cursor = first;
            while (static_cast<unsigned>(*cursor - '0') < 10) {
                value = value * 10 + static_cast<unsigned>(*cursor - '0');
                ++cursor;
            }
            position_ += static_cast<std::size_t>(cursor - first);
            if (position_ != size_) break;
            c = peek();
            if (c == EOF) break;
        }
#else
        while (c >= '0' && c <= '9') {
            previous = value;
            last_digit = static_cast<unsigned>(c - '0');
            if (digits <= 20) ++digits;
            value = value * 10 + static_cast<unsigned>(c - '0');
            ++position_;
            c = peek();
        }
#endif
#ifndef SKIP_RUNTIME_CHECKS
        constexpr auto maximum = std::numeric_limits<std::uint64_t>::max();
        if ((c != EOF && c > ' ') || digits > 20
            || (digits == 20 && (previous > maximum / 10
                || (previous == maximum / 10 && last_digit > maximum % 10)))) {
            good_ = false;
            return *this;
        }
        std::uint64_t limit = static_cast<std::uint64_t>(std::numeric_limits<T>::max());
        if constexpr (std::is_signed_v<T>) limit += negative;
        else if (negative && value != 0) good_ = false;
        if (value > limit) good_ = false;
        if (!good_) return *this;
#endif
        if constexpr (std::is_signed_v<T>) {
            constexpr std::uint64_t minimum_magnitude =
                static_cast<std::uint64_t>(std::numeric_limits<T>::max()) + 1;
            // -(最小値)は評価せず、絶対値が最大値+1のときだけ最小値を使う。
            result = negative
                ? (value == minimum_magnitude ? std::numeric_limits<T>::min()
                                  : static_cast<T>(-static_cast<T>(value)))
                : static_cast<T>(value);
        } else result = static_cast<T>(value);
        return *this;
    }

    VerifyInput& operator>>(char& result) {
        if (!good_) return *this;
        const int c = skip_space();
        if (c != EOF) {
            result = static_cast<char>(c);
            ++position_;
        }
        return *this;
    }

    VerifyInput& operator>>(std::string& result) {
        if (!good_) return *this;
        int c = skip_space();
        if (c == EOF) return *this;
        result.clear();
        for (;;) {
            const std::size_t first = position_;
            while (position_ < size_ && buffer_[position_] > ' ') ++position_;
            result.append(reinterpret_cast<const char*>(buffer_.data() + first), position_ - first);
            if (position_ != size_ || peek() == EOF) break;
        }
        return *this;
    }

    template<class T> requires std::is_floating_point_v<T>
    VerifyInput& operator>>(T& result) {
        std::string token;
        *this >> token;
        if (!good_) return *this;
#ifndef SKIP_RUNTIME_CHECKS
        char* end = nullptr;
        const long double value = std::strtold(token.c_str(), &end);
        if (end != token.c_str() + token.size()) good_ = false;
        else result = static_cast<T>(value);
#else
        result = static_cast<T>(std::strtold(token.c_str(), nullptr));
#endif
        return *this;
    }
};

// VerifyOutput(file=stdout): 非所有のFILEへ書く。破棄時にflushし、FILEは閉じない。
// << 整数/char/文字列/浮動小数点: 値を出力する。整数は64bit以下に対応する。
// fixed(digits): 浮動小数点を小数点以下digits桁で出力する（digits>=0）。
// flush(): バッファの内容をFILEへ渡す。bool / !で書き込み失敗を確認できる。
class VerifyOutput {
    static constexpr std::size_t buffer_size = 1 << 17;
    std::FILE* file_;
    std::array<char, buffer_size> buffer_;
    std::size_t size_ = 0;
    int precision_ = 6;
    bool fixed_ = false, good_ = true;

    void write(std::string_view text) {
        while (!text.empty() && good_) {
            if (size_ == buffer_.size()) flush();
            const auto count = std::min(text.size(), buffer_.size() - size_);
            std::memcpy(buffer_.data() + size_, text.data(), count);
            size_ += count;
            text.remove_prefix(count);
        }
    }

public:
    explicit VerifyOutput(std::FILE* file = stdout) : file_(file) {}
    VerifyOutput(const VerifyOutput&) = delete;
    VerifyOutput& operator=(const VerifyOutput&) = delete;
    ~VerifyOutput() { flush(); }

    explicit operator bool() const noexcept { return good_; }
    bool operator!() const noexcept { return !good_; }

    VerifyOutput& fixed(int digits) {
        if (digits < 0) throw std::invalid_argument("negative output precision");
        fixed_ = true;
        precision_ = digits;
        return *this;
    }

    void flush() noexcept {
        std::size_t written = 0;
        while (written < size_ && good_) {
            const auto count = std::fwrite(buffer_.data() + written, 1, size_ - written, file_);
            if (count == 0) good_ = false;
            written += count;
        }
        size_ = 0;
    }

    template<class T>
        requires (std::is_integral_v<T> && sizeof(T) <= sizeof(std::uint64_t)
                  && !std::is_same_v<T, char>)
    VerifyOutput& operator<<(T value) {
        if constexpr (std::is_same_v<T, bool>) return *this << static_cast<char>('0' + value);
        else {
            constexpr std::size_t capacity = std::numeric_limits<std::uint64_t>::digits10 + 3;
            if (buffer_.size() - size_ < capacity) flush();
            if (!good_) return *this;
            const auto end = std::to_chars(buffer_.data() + size_, buffer_.data() + size_ + capacity, value).ptr;
            size_ = static_cast<std::size_t>(end - buffer_.data());
            return *this;
        }
    }

    VerifyOutput& operator<<(char value) {
        if (size_ == buffer_.size()) flush();
        if (good_) buffer_[size_++] = value;
        return *this;
    }

    VerifyOutput& operator<<(std::string_view value) { write(value); return *this; }
    VerifyOutput& operator<<(const std::string& value) { write(value); return *this; }
    VerifyOutput& operator<<(const char* value) { write(value); return *this; }

    template<class T> requires std::is_floating_point_v<T>
    VerifyOutput& operator<<(T value) {
        char local[128];
        const char* format = fixed_ ? "%.*Lf" : "%.*Lg";
        const int count = std::snprintf(local, sizeof(local), format, precision_,
                                        static_cast<long double>(value));
        if (count < 0) good_ = false;
        else if (static_cast<std::size_t>(count) < sizeof(local))
            write(std::string_view(local, static_cast<std::size_t>(count)));
        else {
            std::vector<char> text(static_cast<std::size_t>(count) + 1);
            std::snprintf(text.data(), text.size(), format, precision_, static_cast<long double>(value));
            write(std::string_view(text.data(), static_cast<std::size_t>(count)));
        }
        return *this;
    }
};

inline VerifyInput verify_in;
inline VerifyOutput verify_out;
#endif

#include <algorithm>
#include <cstdint>
#include <vector>

int main() {
    int n{}, m{}; std::int64_t price{}; verify_in >> n >> m >> price;
    WGraph graph(n);
    while (m--) {
        int a{}, b{}; std::int64_t coins{}; verify_in >> a >> b >> coins;
        graph[a - 1].emplace_back(b - 1, price - coins);
    }
    std::vector<std::int64_t> distance;
    BellmanFord(graph, distance, 0);
    if (distance[n - 1] == -kInf) verify_out << -1 << '\n';
    else verify_out << std::max(std::int64_t{0}, -distance[n - 1]) << '\n';
}
