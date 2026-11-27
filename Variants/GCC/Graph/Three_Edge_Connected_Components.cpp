// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Three_Edge_Connected_Components.cpp
// 参考（DFS木辺・後退辺と2辺カット）: https://doi.org/10.4230/LIPIcs.SEA.2021.20
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/three_edge_connected_components.md
//
// API一覧
// three_edge_connected_component_ids(n,edges): countと、頂点ごとの成分番号idsを返す。
// three_edge_connected_components(n,edges): 各成分の頂点列を返す。
// 元の無向グラフで任意の2辺を除いても互いに到達できる頂点をまとめる（除く辺は0～2本）。
// 頂点・成分番号は0始まり。多重辺を1本ずつ扱い、自己辺は無視する。空グラフの成分数は0。
// どちらも決定的なO((n+m) log(n+1))時間・O(n+m)領域。再帰を使わない。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_THREE_EDGE_CONNECTED_COMPONENTS_DEFINED
#define SIRKIT_THREE_EDGE_CONNECTED_COMPONENTS_DEFINED

struct ThreeEdgeComponentIds
{
    int count;
    std::vector<int> ids;
};

namespace detail
{
struct ThreeEdgeDfs
{
    std::vector<int> parent;
    std::vector<int> depth;
    std::vector<int> order;
    std::vector<std::ptrdiff_t> crossing;
    std::vector<int> highest;
};

// 処理済みの木辺を飛び越え、最初の未割り当ての祖先を求める。
class ThreeEdgeAncestors
{
    std::vector<int> links_;
    std::vector<int> top_;

    int root(int v)
    {
        while (links_[v] >= 0)
        {
            const int p = links_[v];
            if (links_[p] >= 0)
            {
                links_[v] = links_[p];
            }
            v = links_[v];
        }
        return v;
    }

public:
    explicit ThreeEdgeAncestors(int n) : links_(n, -1), top_(n)
    {
        std::iota(top_.begin(), top_.end(), 0);
    }

    int first(int v)
    {
        return top_[root(v)];
    }

    void erase(int v, int above)
    {
        int from = root(v), to = root(above);
        const int top = top_[to];
        if (links_[from] > links_[to])
        {
            std::swap(from, to);
        }
        links_[from] += links_[to];
        links_[to] = from;
        top_[from] = top;
    }
};

inline std::vector<bool> three_edge_spanning_forest(
    int n, std::span<const std::pair<int, int>> edges, ThreeEdgeDfs &dfs)
{
    constexpr int kUnseen = -2;
    std::vector<bool> tree(edges.size());
    std::vector<std::vector<std::pair<int, std::size_t>>> graph(n);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &[u, v] = edges[i];
#ifndef SKIP_RUNTIME_CHECKS
        if (u < 0 || u >= n || v < 0 || v >= n)
        {
            throw std::out_of_range("three_edge_connected_components: edge endpoint out of range");
        }
#endif
        if (u != v)
        {
            graph[u].emplace_back(v, i);
            graph[v].emplace_back(u, i);
        }
    }
    std::vector<std::size_t> cursor(n);
    std::vector<int> path;
    path.reserve(n);
    for (int root = 0; root < n; ++root)
    {
        if (dfs.parent[root] != kUnseen)
        {
            continue;
        }
        dfs.parent[root] = -1;
        dfs.order.push_back(root);
        path.push_back(root);
        while (!path.empty())
        {
            const int v = path.back();
            if (cursor[v] == graph[v].size())
            {
                path.pop_back();
                continue;
            }
            const auto &[to, edge_id] = graph[v][cursor[v]++];
            if (dfs.parent[to] == kUnseen)
            {
                dfs.parent[to] = v;
                dfs.depth[to] = dfs.depth[v] + 1;
                dfs.order.push_back(to);
                tree[edge_id] = true;
                path.push_back(to);
            }
        }
    }
    return tree;
}

inline ThreeEdgeDfs three_edge_dfs(int n, std::span<const std::pair<int, int>> edges)
{
    ThreeEdgeDfs dfs{
        std::vector<int>(n, -2), std::vector<int>(n), {}, std::vector<std::ptrdiff_t>(n), std::vector<int>(n, -1)};
    dfs.order.reserve(n);
    const auto tree = three_edge_spanning_forest(n, edges, dfs);

    std::vector<std::vector<int>> back_descendants(n);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        auto [upper, lower] = edges[i];
        if (tree[i] || upper == lower)
        {
            continue;
        }
        if (dfs.depth[upper] > dfs.depth[lower])
        {
            std::swap(upper, lower);
        }
        ++dfs.crossing[lower];
        --dfs.crossing[upper];
        back_descendants[upper].push_back(lower);
    }
    ThreeEdgeAncestors unassigned(n);
    for (auto it = dfs.order.rbegin(); it != dfs.order.rend(); ++it)
    {
        const int u = *it;
        if (dfs.parent[u] >= 0)
        {
            dfs.crossing[dfs.parent[u]] += dfs.crossing[u];
        }
        // 同じ木辺をまたぐ後退辺の上端は、逆DFS順で深い方から現れる。
        for (const int descendant : back_descendants[u])
        {
            for (int v = unassigned.first(descendant); dfs.depth[v] > dfs.depth[u]; v = unassigned.first(v))
            {
                dfs.highest[v] = dfs.depth[u];
                unassigned.erase(v, dfs.parent[v]);
            }
        }
    }
    return dfs;
}

struct ThreeEdgeCutGroups
{
    std::vector<int> coordinate;
    std::vector<int> period;
};

inline ThreeEdgeCutGroups three_edge_cut_groups(const ThreeEdgeDfs &dfs, std::size_t edge_count)
{
    const auto n = dfs.order.size();
    ThreeEdgeCutGroups result{std::vector<int>(n, -1), {}};
    std::vector<int> previous(n, -1), last(edge_count + 1, -1), path;
    path.reserve(n);
    for (const int v : dfs.order)
    {
        while (!path.empty() && path.back() != dfs.parent[v])
        {
            const int u = path.back();
            path.pop_back();
            if (dfs.parent[u] >= 0 && dfs.crossing[u] > 0)
            {
                last[dfs.crossing[u]] = previous[u];
            }
        }
        if (dfs.parent[v] >= 0)
        {
            const auto size = dfs.crossing[v];
            const int nearest = last[size];
            // 本数が等しく、深い側の後退辺がすべて浅い木辺もまたぐなら、後退辺の集合が一致する。
            if (nearest >= 0 && dfs.highest[v] < dfs.depth[nearest])
            {
                result.coordinate[v] = result.coordinate[nearest];
                ++result.period[result.coordinate[v]];
            }
            else
            {
                result.coordinate[v] = static_cast<int>(result.period.size());
                // 橋は2分割。後退辺1本の群も、木辺の本数より分割が1個多い。
                result.period.push_back(size <= 1 ? 2 : 1);
            }
            if (size > 0)
            {
                previous[v] = nearest;
                last[size] = v;
            }
        }
        path.push_back(v);
    }
    return result;
}

struct ThreeEdgeEvent
{
    std::size_t coordinate;
    std::size_t state;
};

inline void rank_three_edge_events(std::vector<ThreeEdgeEvent> &events, std::size_t coordinates)
{
    const auto size = events.size();
    std::vector<std::size_t> states(coordinates + 1), first(size), second(size), offsets(size + 1);
    std::vector<std::array<std::size_t, 2>> keys(size);
    std::iota(first.begin(), first.end(), std::size_t{0});
    const auto sort_by = [&](int column, std::span<const std::size_t> input, std::span<std::size_t> output)
    {
        std::fill(offsets.begin(), offsets.end(), 0);
        for (const auto i : input)
        {
            ++offsets[keys[i][column]];
        }
        std::size_t prefix = 0;
        for (auto &value : offsets)
        {
            const auto count = value;
            value = prefix;
            prefix += count;
        }
        for (const auto i : input)
        {
            output[offsets[keys[i][column]]++] = i;
        }
    };
    for (; coordinates > 1; coordinates = (coordinates + 1) / 2)
    {
        std::fill_n(states.begin(), coordinates + 1, 0);
        for (std::size_t i = 0; i < size; ++i)
        {
            auto &[position, state] = events[i];
            states[position] = state;
            const auto left = position & ~std::size_t{1};
            keys[i] = {states[left], states[left + 1]};
            position /= 2;
        }
        // 片方ずつ計数ソートして同じ組に同じ番号を割り当てる。全ゼロの状態は0のまま。
        sort_by(1, first, second);
        sort_by(0, second, first);
        std::array<std::size_t, 2> previous{};
        std::size_t rank = 0;
        for (const auto i : first)
        {
            if (keys[i] != previous)
            {
                previous = keys[i];
                ++rank;
            }
            events[i].state = rank;
        }
    }
}

inline ThreeEdgeComponentIds three_edge_profiles(const ThreeEdgeDfs &dfs, const ThreeEdgeCutGroups &groups)
{
    const auto n = dfs.order.size();
    std::vector<ThreeEdgeEvent> events;
    events.reserve(2 * n);
    std::vector<int> value(groups.period.size()), path;
    path.reserve(n);
    std::vector<std::size_t> active_event(n);
    const auto change = [&](int v, bool enter)
    {
        const int coordinate = groups.coordinate[v];
        if (coordinate < 0 || groups.period[coordinate] == 1)
        {
            return;
        }
        const int period = groups.period[coordinate];
        auto &state = value[coordinate];
        if (enter)
        {
            ++state;
            if (state == period)
            {
                state = 0;
            }
        }
        else
        {
            if (state == 0)
            {
                state = period;
            }
            --state;
        }
        events.push_back({static_cast<std::size_t>(coordinate), static_cast<std::size_t>(state)});
    };
    for (const int v : dfs.order)
    {
        while (!path.empty() && path.back() != dfs.parent[v])
        {
            change(path.back(), false);
            path.pop_back();
        }
        change(v, true);
        path.push_back(v);
        active_event[v] = events.size();
    }
    // 最後に残った祖先列は、この後に照会する頂点がないためイベントを戻す必要がない。
    rank_three_edge_events(events, groups.period.size());

    std::vector<int> component(events.size() + 1, -1), id(events.size() + 1);
    ThreeEdgeComponentIds result{0, std::vector<int>(n)};
    int root = -1;
    for (const int v : dfs.order)
    {
        if (dfs.parent[v] < 0)
        {
            root = v;
        }
        const auto time = active_event[v];
        const auto state = time == 0 ? 0 : events[time - 1].state;
        if (component[state] != root)
        {
            component[state] = root;
            id[state] = result.count++;
        }
        result.ids[v] = id[state];
    }
    return result;
}
} // namespace detail

inline ThreeEdgeComponentIds three_edge_connected_component_ids(
    int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0)
    {
        throw std::length_error("three_edge_connected_components: negative vertex count");
    }
    const auto dfs = detail::three_edge_dfs(n, edges);
    const auto groups = detail::three_edge_cut_groups(dfs, edges.size());
    return detail::three_edge_profiles(dfs, groups);
}

inline std::vector<std::vector<int>> three_edge_connected_components(
    int n, std::span<const std::pair<int, int>> edges)
{
    const auto [count, ids] = three_edge_connected_component_ids(n, edges);
    std::vector<std::vector<int>> result(count);
    for (int v = 0; v < n; ++v)
    {
        result[ids[v]].push_back(v);
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
