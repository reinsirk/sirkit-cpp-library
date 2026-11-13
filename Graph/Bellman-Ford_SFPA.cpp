// SPDX-License-Identifier: CC0-1.0
// 参考: https://zenn.dev/reputeless/books/standard-cpp-for-competitive-programming/viewer/bellman-ford
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/negative_shortest_paths.md
//
// API一覧
// [共通型・定数]
// Edge(to,w): 行先toとint64_tの重みwを持つ有向辺。
// WGraph: 出発点を隣接リストの添字とする辺列。INF=std::int64_t{1}<<60。
//
// [SPFA]
// SPFA(graph, distances, start_index): 始点からの最短距離をdistancesに格納し、到達可能な負閉路があればtrueを返す。
// SKIP_RUNTIME_CHECKSを定義すると入力検査を省く。
// 検査は始点と処理中に参照する辺のみ。未到達成分の出辺は走査しない。
// 通常は更新頂点をFIFO処理し、長い反復は到達部分の強連結成分ごとに解く。
// 標準C++23、重みは[-1e9, 1e9]。
// 到達可能な負閉路があればtrue。距離は成功時に全上書きし、未到達INF・負影響-INF。
// 最悪O(VE+V+E)時間・O(V)追加領域。有限距離のINF/-INF衝突はoverflow_error。

#include <algorithm>
#include <array>
#include <span>
#include <type_traits>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>
#include <stdexcept>
#include <vector>

#ifndef SIRKIT_INF_DEFINED
#define SIRKIT_INF_DEFINED
inline constexpr std::int64_t INF = std::int64_t{1} << 60;
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

#ifndef SIRKIT_SPFA_COMPONENTS_DEFINED
#define SIRKIT_SPFA_COMPONENTS_DEFINED
namespace reinsirk::detail {
// 遅い反復だけを到達部分の強連結成分ごとに解く。追加領域O(V)。
#if defined(__GNUC__) || defined(__clang__)
[[gnu::noinline]]
#endif
inline void SPFAComponents(const WGraph& graph, std::span<const int> sources,
    std::vector<std::int64_t>& distance, std::int64_t unreachable, std::int64_t lower_bound,
    std::vector<int>& negative
#ifndef SKIP_RUNTIME_CHECKS
    , std::vector<std::uint8_t>& checked
#endif
) {
#ifndef SKIP_RUNTIME_CHECKS
    const int n = static_cast<int>(graph.size());
#endif
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
#ifndef SKIP_RUNTIME_CHECKS
                if (!checked[from]) {
                    if (edge.to < 0 || edge.to >= n)
                        throw std::out_of_range("SPFA: invalid destination vertex");
                    if (edge.w < -1'000'000'000 || edge.w > 1'000'000'000)
                        throw std::out_of_range("SPFA: cost must be in [-1e9, 1e9]");
                }
#endif
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
#ifndef SKIP_RUNTIME_CHECKS
                checked[from] = 1;
#endif
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


// 通常のFIFO処理が長引いた場合だけ、温存した距離から残りを解く。
#if defined(__GNUC__) || defined(__clang__)
[[gnu::noinline, gnu::cold]]
#endif
inline void SPFAFinishComponents(const WGraph& graph,
    std::vector<std::int64_t>& distance, std::int64_t unreachable,
    std::vector<int>& affected, std::vector<std::uint8_t>& state) {
    const auto lower_bound = -static_cast<std::int64_t>(graph.size()) * 1'000'000'000;
    for (const int vertex : affected) distance[vertex] = lower_bound;
    std::vector<int> sources;
    sources.reserve(graph.size());
#ifndef SKIP_RUNTIME_CHECKS
    std::vector<std::uint8_t> checked(graph.size());
#endif
    for (std::size_t vertex = 0; vertex < graph.size(); ++vertex) {
        if (!graph[vertex].empty()) sources.push_back(static_cast<int>(vertex));
#ifndef SKIP_RUNTIME_CHECKS
        checked[vertex] = (state[vertex] & 2) != 0;
#endif
    }
    SPFAComponents(graph, sources, distance, unreachable, lower_bound, affected
#ifndef SKIP_RUNTIME_CHECKS
        , checked
#endif
    );
    for (const int vertex : affected) {
        state[vertex] |= 4;
        distance[vertex] = -INF;
    }
}
} // namespace reinsirk::detail
#endif

// 64頂点以下はキュー・補助状態をスタックに置く。
#ifndef SIRKIT_SPFA_DEFINED
#define SIRKIT_SPFA_DEFINED
namespace reinsirk::detail {
template <bool Small, bool Bounded>
inline bool SPFARun(const WGraph &graph, std::vector<std::int64_t> &distances,
                 int start_index)
{
    constexpr std::int64_t max_cost = 1'000'000'000;
    if (graph.size() > static_cast<std::size_t>(std::numeric_limits<int>::max()))
        throw std::length_error("SPFA: too many vertices");
    const int n = static_cast<int>(graph.size());
#ifndef SKIP_RUNTIME_CHECKS
    if (start_index < 0 || start_index >= n)
        throw std::out_of_range("SPFA: invalid start vertex");
#endif

    if (graph[start_index].empty()) {
#ifndef SKIP_RUNTIME_CHECKS
        std::vector<std::int64_t> result(graph.size(), INF);
        result[start_index] = 0;
        distances.swap(result);
#else
        distances.assign(graph.size(), INF);
        distances[start_index] = 0;
#endif
        return false;
    }

    // n*最大辺重み<INFなら、内部値と出力値の変換が要らない。
    const std::int64_t unreachable = Bounded ? INF : (static_cast<std::int64_t>(n) + 1) * max_cost;
    std::vector<std::int64_t> local_distance;
#ifndef SKIP_RUNTIME_CHECKS
    auto& distance = local_distance;
#else
    auto& distance = Small ? distances : local_distance;
#endif
    distance.assign(graph.size(), unreachable);
    std::conditional_t<Small, std::array<int, 64>, std::vector<int>> path_edges;
    if constexpr (Small) path_edges.fill(0);
    else path_edges.assign(graph.size(), 0);
    std::vector<int> affected;
    // stateのbit 0はキュー内、bit 1は初回検査済み、bit 2は負閉路の影響。
    constexpr std::uint8_t queued_bit = 1;
#ifndef SKIP_RUNTIME_CHECKS
    constexpr std::uint8_t checked_bit = 2;
#endif
    constexpr std::uint8_t negative_bit = 4;
    std::conditional_t<Small, std::array<std::uint8_t, 64>, std::vector<std::uint8_t>> state;
    if constexpr (Small) state.fill(0);
    else state.assign(graph.size(), 0);
    // 同時にキューに入る頂点はV個以下。書いた要素だけ読むため初期化は不要。
    const std::size_t queue_capacity = graph.size();
    std::array<int, Small ? 64 : 0> local_queue;
    std::unique_ptr<int[]> queue_owner;
    int* queue;
    if constexpr (Small) queue = local_queue.data();
    else {
        queue_owner = std::make_unique_for_overwrite<int[]>(queue_capacity);
        queue = queue_owner.get();
    }
    std::size_t head = 0, tail = 0, queued = 0;
    const auto enqueue = [&](int vertex)
    {
        queue[tail] = vertex;
        if (++tail == queue_capacity)
            tail = 0;
        ++queued;
    };
    distance[start_index] = 0;
    enqueue(start_index);
    state[start_index] = queued_bit;

    // 負閉路の下流を一度だけ走査し、以後は距離の緩和から除外する。
    const auto mark_negative = [&](int seed)
    {
        if ((state[seed] & negative_bit))
            return;
        // 負閉路が見つかるまでは、影響頂点の領域を確保しない。
        if (affected.empty())
            affected.reserve(graph.size());
        const std::size_t first = affected.size();
        state[seed] |= negative_bit;
        if constexpr (Bounded) distance[seed] = -INF;
        affected.push_back(seed);
        for (std::size_t affected_head = first; affected_head < affected.size(); ++affected_head)
        {
            for (const auto &edge : graph[affected[affected_head]])
            {
#ifndef SKIP_RUNTIME_CHECKS
                // 初回検査の途中から来る場合もあるため、伝播側でも検査する。
                if (edge.to < 0 || edge.to >= n)
                    throw std::out_of_range("SPFA: invalid destination vertex");
                if (edge.w < -max_cost || edge.w > max_cost)
                    throw std::out_of_range("SPFA: cost must be in [-1e9, 1e9]");
#endif
                if ((state[edge.to] & negative_bit))
                    continue;
                state[edge.to] |= negative_bit;
                if constexpr (Bounded) distance[edge.to] = -INF;
                affected.push_back(edge.to);
            }
        }
    };

    // 短い処理ではFIFOを保ち、繰返しが長い場合だけ成分ごとに確定する。
    const auto budget_limit = std::numeric_limits<std::size_t>::max();
    std::size_t component_budget = graph.size() <= budget_limit / 16 ? graph.size() * 16 : budget_limit;
    std::int64_t current_distance = 0;
    int current_length = 0;
    // 通常はfromが負影響ならtrue。Directでは未キューの更新先、なければ-1。
#if defined(__clang__)
    const auto relax_edge = [&]<bool Single = false, bool Direct = false>(int from, const Edge &edge)
        -> std::conditional_t<Direct, int, bool>
#else
    const auto relax_edge = [&]<bool Single = false>(int from, const Edge &edge)
#endif
    {
#if defined(__clang__)
        constexpr std::conditional_t<Direct, int, bool> unchanged = Direct ? -1 : 0;
#else
        constexpr bool unchanged = false;
#endif
        // Boundedでは候補>-INFなので、負影響先は距離比較だけで除外できる。
        if constexpr (!Bounded) if (state[edge.to] & negative_bit)
            return unchanged;
        // 格納する経路はn辺未満なので、候補の加算もint64_t内に収まる。
        const std::int64_t candidate = current_distance + edge.w;
        if (candidate >= distance[edge.to])
            return unchanged;
        const int length = current_length;
        // 複数辺では負自己ループを即時に処理し、保持した始点距離を使い続けない。
        if (length >= n || (!Single && edge.to == from)) [[unlikely]]
        {
            mark_negative(edge.to);
#if defined(__clang__)
            if constexpr (!Direct) if (state[from] & negative_bit)
                return true;
#else
            if ((state[from] & negative_bit))
                return true;
#endif
            return unchanged;
        }
        distance[edge.to] = candidate;
        path_edges[edge.to] = length;
        if (!(state[edge.to] & queued_bit))
        {
#if defined(__clang__)
            if constexpr (Direct) return edge.to;
            else
#endif
            {
                enqueue(edge.to);
                state[edge.to] |= queued_bit;
            }
        }
        return unchanged;
    };

    while (queued != 0)
    {
#if defined(__clang__)
        std::conditional_t<Small, const int, int> from = queue[head];
#else
        const int from = queue[head];
#endif
        if (++head == queue_capacity)
            head = 0;
        --queued;
#if defined(__clang__)
    process_vertex:
#endif
        const std::uint8_t vertex_state = state[from];
        if (vertex_state & negative_bit)
            continue;
        if constexpr (!Small) {
            if (component_budget == 0) [[unlikely]] {
                SPFAFinishComponents(graph, distance, unreachable, affected, state);
                break;
            }
            --component_budget;
        }
#ifndef SKIP_RUNTIME_CHECKS
        state[from] = checked_bit;
#else
        state[from] = 0;
#endif
        current_distance = distance[from];
        current_length = path_edges[from] + 1;
#ifndef SKIP_RUNTIME_CHECKS
        const bool validate = !(vertex_state & checked_bit);
#endif
        // 1辺ならループと途中の自己辺更新への対処を省ける。
        if (graph[from].size() == 1) {
            const auto& edge = graph[from].front();
#ifndef SKIP_RUNTIME_CHECKS
            if (validate) {
                if (edge.to < 0 || edge.to >= n)
                    throw std::out_of_range("SPFA: invalid destination vertex");
                if (edge.w < -max_cost || edge.w > max_cost)
                    throw std::out_of_range("SPFA: cost must be in [-1e9, 1e9]");
            }
#endif
#if defined(__clang__)
            // 待機頂点がなければ直接進む。小規模は従来のコードを保つ。
            if constexpr (!Small) {
                if (queued == 0) {
                    const int next = relax_edge.template operator()<true, true>(from,edge);
                    if (next >= 0) {
                        from = next;
                        goto process_vertex;
                    }
                } else relax_edge.template operator()<true>(from,edge);
            } else
#endif
            relax_edge.template operator()<true>(from,edge);
            continue;
        }
#ifndef SKIP_RUNTIME_CHECKS
        if (validate)
        {
            for (const auto &edge : graph[from])
            {
                if (edge.to < 0 || edge.to >= n)
                    throw std::out_of_range("SPFA: invalid destination vertex");
                if (edge.w < -max_cost || edge.w > max_cost)
                    throw std::out_of_range("SPFA: cost must be in [-1e9, 1e9]");
                if (relax_edge(from, edge))
                    break;
            }
        }
        else
#endif
        {
            for (const auto &edge : graph[from])
                if (relax_edge(from, edge))
                    break;
        }
    }

    if constexpr (!Bounded) for (int vertex = 0; vertex < n; ++vertex)
    {
        if ((state[vertex] & negative_bit))
            distance[vertex] = -INF;
        else if (distance[vertex] == unreachable)
            distance[vertex] = INF;
        else if (distance[vertex] <= -INF || distance[vertex] >= INF)
            throw std::overflow_error("SPFA: finite distance collides with INF/-INF");
    }
#ifndef SKIP_RUNTIME_CHECKS
    distances.swap(distance);
#else
    if constexpr (!Small) distances.swap(distance);
#endif
    return !affected.empty();
}
} // namespace reinsirk::detail

inline bool SPFA(const WGraph& graph, std::vector<std::int64_t>& distances, int start_index) {
    if (graph.size() <= 64) return reinsirk::detail::SPFARun<true, true>(graph, distances, start_index);
    if (graph.size() <= static_cast<std::size_t>((INF - 1) / 1'000'000'000))
        return reinsirk::detail::SPFARun<false, true>(graph, distances, start_index);
    return reinsirk::detail::SPFARun<false, false>(graph, distances, start_index);
}
#endif
