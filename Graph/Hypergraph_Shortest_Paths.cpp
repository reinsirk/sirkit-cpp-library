// SPDX-License-Identifier: CC0-1.0
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/hypergraph_shortest_paths.md
//
// API一覧
// Hyperedge{members,cost=1}: 同じ集合の任意のメンバー間を移動する費用。membersはspan<const int>。
// HypergraphShortestPaths(n,groups): n頂点の集合間移動を用意。頂点は0始まり。入力はコピーする。
//   集合数をG、全メンバー数をLとし、構築 O(n+G+L) 時間・領域。
// unit_distances(source,adjacent={}): 集合・通常辺とも費用1。未到達は-1。O(n+m+G+L)。
//   adjacent(v,visit)は通常の行き先ごとにvisit(to)を呼ぶ。省略時は通常辺なし。
// weighted_distances(source,adjacent={}): 非負uint64_t費用。未到達・範囲超過はnullopt。
//   adjacent(v,visit)は各辺についてvisit(to,cost)を呼ぶ。省略時は通常辺なし。
//   通常辺数をmとし、O(n+G+(m+L)log(m+L+2)) 時間。両照会ともO(n+G)領域、重み付きはさらにO(m+L)。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_HYPERGRAPH_SHORTEST_PATHS_DEFINED
#define SIRKIT_HYPERGRAPH_SHORTEST_PATHS_DEFINED

struct Hyperedge
{
    std::span<const int> members;
    std::uint64_t cost = 1;
};

class HypergraphShortestPaths
{
    struct NoEdges
    {
        void operator()(int, auto &&) const
        {
        }
    };

public:
    HypergraphShortestPaths(int n, std::span<const Hyperedge> groups)
    {
        if (n < 0)
        {
            throw std::length_error("HypergraphShortestPaths: negative vertex count");
        }
        heads_.assign(n, kEnd);
        groups_.reserve(groups.size());
        std::size_t total = 0;
        for (const auto &group : groups)
        {
            total += group.members.size();
        }
        membership_.reserve(total);
        for (const auto &group : groups)
        {
            const auto first = membership_.size();
            const auto id = groups_.size();
            for (const int v : group.members)
            {
                check_vertex(v);
                membership_.push_back({heads_[v], id, v});
                heads_[v] = membership_.size() - 1;
            }
            groups_.push_back({first, membership_.size(), group.cost});
            unit_ = unit_ && group.cost == 1;
        }
    }

    template <typename Adjacent = NoEdges>
    [[nodiscard]] std::vector<int> unit_distances(int source, Adjacent &&adjacent = {}) const
    {
        check_vertex(source);
        if (!unit_)
        {
            throw std::invalid_argument("HypergraphShortestPaths: unit group cost required");
        }
        std::vector<int> distance(heads_.size(), -1);
        std::vector<int> pending;
        pending.reserve(heads_.size());
        std::vector<bool> active(groups_.size());
        distance[source] = 0;
        pending.push_back(source);
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const int v = pending[i];
            const int next = distance[v] + 1;
            auto relax = [&](int to)
            {
                check_vertex(to);
                if (distance[to] == -1)
                {
                    distance[to] = next;
                    pending.push_back(to);
                }
            };
            std::invoke(adjacent, v, relax);
            each_group(v, active, [&](const Group &group)
                       {
                           for (auto id = group.first; id < group.last; ++id)
                           {
                               relax(membership_[id].vertex);
                           }
                       });
        }
        return distance;
    }

    template <typename Adjacent = NoEdges>
    [[nodiscard]] std::vector<std::optional<std::uint64_t>> weighted_distances(int source, Adjacent &&adjacent = {}) const
    {
        check_vertex(source);
        using Entry = std::pair<std::uint64_t, int>;
        std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pending;
        std::vector<std::optional<std::uint64_t>> distance(heads_.size());
        std::vector<bool> active(groups_.size());
        distance[source] = 0;
        pending.emplace(0, source);
        while (!pending.empty())
        {
            const auto [current, v] = pending.top();
            pending.pop();
            if (current != *distance[v])
            {
                continue;
            }
            auto relax = [&](int to, std::uint64_t cost)
            {
                check_vertex(to);
                if (cost > std::numeric_limits<std::uint64_t>::max() - current)
                {
                    return;
                }
                const auto next = current + cost;
                if (!distance[to] || next < *distance[to])
                {
                    distance[to] = next;
                    pending.emplace(next, to);
                }
            };
            std::invoke(adjacent, v, relax);
            each_group(v, active, [&](const Group &group)
                       {
                           for (auto id = group.first; id < group.last; ++id)
                           {
                               relax(membership_[id].vertex, group.cost);
                           }
                       });
        }
        return distance;
    }

private:
    struct Member
    {
        std::size_t previous;
        std::size_t group;
        int vertex;
    };
    struct Group
    {
        std::size_t first;
        std::size_t last;
        std::uint64_t cost;
    };
    static constexpr auto kEnd = std::numeric_limits<std::size_t>::max();

    void check_vertex([[maybe_unused]] int v) const
    {
#ifndef SKIP_RUNTIME_CHECKS
        if (static_cast<std::size_t>(v) >= heads_.size())
        {
            throw std::out_of_range("HypergraphShortestPaths: invalid vertex");
        }
#endif
    }

    template <typename Callback>
    void each_group(int v, std::vector<bool> &active, Callback &&callback) const
    {
        for (auto id = heads_[v]; id != kEnd; id = membership_[id].previous)
        {
            const auto group = membership_[id].group;
            if (!active[group])
            {
                active[group] = true;
                std::invoke(callback, groups_[group]);
            }
        }
    }

    std::vector<std::size_t> heads_;
    std::vector<Member> membership_;
    std::vector<Group> groups_;
    bool unit_ = true;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
