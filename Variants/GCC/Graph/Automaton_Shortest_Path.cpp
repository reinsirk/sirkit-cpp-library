// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Automaton_Shortest_Path.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/automaton_shortest_path.md
//
// API一覧
// AutomatonPathEdge{from,to,label,weight}: 0始まりの端点、任意の int ラベル、非負の uint64_t 重み。
// automaton_shortest_path(n,edges,source,target,state_count,initial,transition,accept,undirected=false):
//   ラベルを順に読んで受理状態で終点へ着く最短歩道。AutomatonPath{distance,vertices,states,edges} を返す。
//   状態は0始まり。transition(state,label) は次状態、-1で通行不可。accept(state) は受理するなら true。
//   edges は入力順の辺ID列。到達不能・距離の uint64_t 範囲超過はnullopt。
//   状態数を S とし、時間 O(S(n+m log(Sm+1)))、領域 O(S(n+m))。全辺重み1なら O(S(n+m)) 時間。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_AUTOMATON_SHORTEST_PATH_DEFINED
#define SIRKIT_AUTOMATON_SHORTEST_PATH_DEFINED

struct AutomatonPathEdge
{
    int from;
    int to;
    int label;
    std::uint64_t weight;
};

struct AutomatonPath
{
    std::uint64_t distance;
    std::vector<int> vertices;
    std::vector<int> states;
    std::vector<std::size_t> edges;
};

template <typename Transition, typename Accept>
std::optional<AutomatonPath> automaton_shortest_path(
    int n, std::span<const AutomatonPathEdge> edges, int source, int target, int state_count,
    int initial, Transition &&transition, Accept &&accept, bool undirected = false)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (source < 0 || source >= n || target < 0 || target >= n)
    {
        throw std::out_of_range("automaton_shortest_path: invalid terminal");
    }
    if (state_count <= 0 || initial < 0 || initial >= state_count)
    {
        throw std::out_of_range("automaton_shortest_path: invalid initial state");
    }
#endif
    struct VertexState
    {
        std::optional<std::uint64_t> distance;
        std::size_t previous = 0;
        std::size_t edge = 0;
    };
    const auto states = static_cast<std::size_t>(state_count);
    std::vector<VertexState> info;
    if (static_cast<std::size_t>(n) > info.max_size() / states)
    {
        throw std::length_error("automaton_shortest_path: too many vertex-state pairs");
    }
    info.resize(static_cast<std::size_t>(n) * states);
    struct Arc
    {
        int to;
        std::size_t id;
    };
    std::vector<std::vector<Arc>> graph(n);
    bool unit = true;
    for (std::size_t id = 0; id < edges.size(); ++id)
    {
        const int from = edges[id].from;
        const int to = edges[id].to;
#ifndef SKIP_RUNTIME_CHECKS
        if (from < 0 || from >= n || to < 0 || to >= n)
        {
            throw std::out_of_range("automaton_shortest_path: invalid endpoint");
        }
#endif
        graph[from].push_back({to, id});
        if (undirected && from != to)
        {
            graph[to].push_back({from, id});
        }
        unit = unit && edges[id].weight == 1;
    }
    auto index = [states](int vertex, int state)
    {
        return static_cast<std::size_t>(vertex) * states + state;
    };
    const auto start = index(source, initial);
    auto make_path = [&](std::size_t goal, std::uint64_t distance)
    {
        AutomatonPath result{distance, {}, {}, {}};
        for (auto where = goal;; where = info[where].previous)
        {
            result.vertices.push_back(static_cast<int>(where / states));
            result.states.push_back(static_cast<int>(where % states));
            if (where == start)
            {
                break;
            }
            result.edges.push_back(info[where].edge);
        }
        std::ranges::reverse(result.vertices);
        std::ranges::reverse(result.states);
        std::ranges::reverse(result.edges);
        return result;
    };
    auto search = [&]<bool Unit>() -> std::optional<AutomatonPath>
    {
        using Entry = std::pair<std::uint64_t, std::size_t>;
        using Queue = std::conditional_t<Unit, std::queue<Entry>, std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>>>;
        Queue pending;
        info[start].distance = 0;
        pending.emplace(0, start);
        while (!pending.empty())
        {
            Entry current;
            if constexpr (Unit)
            {
                current = pending.front();
            }
            else
            {
                current = pending.top();
            }
            const auto [distance, where] = current;
            pending.pop();
            if constexpr (!Unit)
            {
                if (distance != *info[where].distance)
                {
                    continue;
                }
            }
            const int vertex = static_cast<int>(where / states);
            const int state = static_cast<int>(where % states);
            if (vertex == target && std::invoke(accept, state))
            {
                return make_path(where, distance);
            }
            for (const auto &[v, id] : graph[vertex])
            {
                const auto &edge = edges[id];
                const int next_state = std::invoke(transition, state, edge.label);
                if (next_state == -1)
                {
                    continue;
                }
#ifndef SKIP_RUNTIME_CHECKS
                if (next_state < 0 || next_state >= state_count)
                {
                    throw std::out_of_range("automaton_shortest_path: invalid transition result");
                }
#endif
                if constexpr (!Unit)
                {
                    if (edge.weight > std::numeric_limits<std::uint64_t>::max() - distance)
                    {
                        continue;
                    }
                }
                const auto next = index(v, next_state);
                const auto total = distance + edge.weight;
                auto &entry = info[next];
                if (!entry.distance || total < *entry.distance)
                {
                    entry = {total, where, id};
                    pending.emplace(total, next);
                }
            }
        }
        return std::nullopt;
    };
    return unit ? search.template operator()<true>() : search.template operator()<false>();
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
