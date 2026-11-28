// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Shortest_Odd_Cycle.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/shortest_odd_cycle.md
//
// API一覧
// shortest_odd_cycle(n,edges):
//   無向グラフで最も短い奇数長の単純閉路を、巡回順の頂点列で返す。なければ空列。
//   頂点番号は0始まり。始点を末尾に重ねない。自己辺は1頂点の閉路、多重辺も可。
//   O(n(n+m))時間、O(n+m)領域。二部グラフの連結成分では全頂点からのBFSを省く。
//   非二部成分の始点には奇数歩で戻れるため、BFSは目標か深さ上限で終わる。
//   最短の奇数長閉歩道に頂点の重複があれば、分割してさらに短い奇数長閉歩道が得られる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <cstddef>
#include <limits>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SHORTEST_ODD_CYCLE_DEFINED
#define SIRKIT_SHORTEST_ODD_CYCLE_DEFINED

[[nodiscard]] inline std::vector<int> shortest_odd_cycle(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("shortest_odd_cycle: negative vertex count");
    }
    std::vector<std::vector<int>> adjacent(n);
    int self_loop = -1;
    for (const auto &[u, v] : edges)
    {
        if (u == v)
        {
            self_loop = u;
        }
        else
        {
            adjacent[u].push_back(v);
            adjacent[v].push_back(u);
        }
    }
    if (self_loop != -1)
    {
        return {self_loop};
    }

    std::vector<int> color(n, -1);
    std::vector<int> nonbipartite;
    std::vector<int> component;
    for (int root = 0; root < n; ++root)
    {
        if (color[root] != -1)
        {
            continue;
        }
        component.clear();
        component.push_back(root);
        color[root] = 0;
        bool odd = false;
        for (std::size_t i = 0; i < component.size(); ++i)
        {
            const int u = component[i];
            for (const int v : adjacent[u])
            {
                if (color[v] == -1)
                {
                    color[v] = color[u] ^ 1;
                    component.push_back(v);
                }
                else
                {
                    odd |= color[v] == color[u];
                }
            }
        }
        if (odd)
        {
            nonbipartite.insert(nonbipartite.end(), component.begin(), component.end());
        }
    }
    if (nonbipartite.empty())
    {
        return {};
    }

    const auto states = static_cast<std::size_t>(n) * 2;
    constexpr auto kUnseen = std::numeric_limits<std::size_t>::max();
    std::vector<std::size_t> previous(states), distance(states), pending;
    std::vector<int> cycle;
    auto limit = static_cast<std::size_t>(n) + 1;
    for (const int source : nonbipartite)
    {
        const auto first = static_cast<std::size_t>(source) * 2;
        const auto last = first + 1;
        std::fill(previous.begin(), previous.end(), kUnseen);
        pending.clear();
        pending.push_back(first);
        previous[first] = first;
        distance[first] = 0;

        for (std::size_t i = 0;; ++i)
        {
            const auto state = pending[i];
            const auto depth = distance[state] + 1;
            if (depth >= limit)
            {
                break;
            }
            const auto parity = (state & 1U) ^ 1U;
            for (const int v : adjacent[state / 2])
            {
                const auto next = static_cast<std::size_t>(v) * 2 + parity;
                if (previous[next] == kUnseen)
                {
                    previous[next] = state;
                    distance[next] = depth;
                    pending.push_back(next);
                    if (next == last)
                    {
                        break;
                    }
                }
            }
            if (previous[last] != kUnseen)
            {
                break;
            }
        }
        if (previous[last] == kUnseen)
        {
            continue;
        }
        limit = distance[last];
        cycle.clear();
        for (auto state = previous[last]; state != first; state = previous[state])
        {
            cycle.push_back(static_cast<int>(state / 2));
        }
        cycle.push_back(source);
        std::ranges::reverse(cycle);
        if (limit == 3)
        {
            break;
        }
    }
    return cycle;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
