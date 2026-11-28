// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Undirected_Walk_Lengths.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/undirected_walk_lengths.md
//
// API一覧
// UndirectedWalkLengths(n,edges,source): 0始まり無向グラフで、sourceから各頂点への偶数・奇数最短長を求める。
//   移動では頂点や辺の再利用が可能。自己辺と多重辺も可。構築O(n+m)時間・領域。
// minimum_lengths(v): {偶数最短長,奇数最短長}。到達不能側は-1。O(1)。
// can_walk_to(v,length): length本の辺でvに到着可能ならtrue。O(1)。
// vertices_on_walks(target,length): そのような移動で通れるすべての頂点を昇順で返す。O(n+m)時間、O(n)追加領域。
//   余り2歩は同じ辺の往復で補える。孤立点からは長さ0だけ可能。
//   targetへ指定の偶奇で到達できれば、beforeの経路を逆にたどることで対応する偶奇のafterも存在。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNDIRECTED_WALK_LENGTHS_DEFINED
#define SIRKIT_UNDIRECTED_WALK_LENGTHS_DEFINED

class UndirectedWalkLengths
{
public:
    UndirectedWalkLengths(int n, std::span<const std::pair<int, int>> edges, int source)
        : source_(source)
    {
        if (n < 0)
        {
            throw std::length_error("UndirectedWalkLengths: negative vertex count");
        }
        adjacency_.resize(n);
        check_vertex(source);
        for (const auto &[u, v] : edges)
        {
            check_vertex(u);
            check_vertex(v);
            adjacency_[u].push_back(v);
            adjacency_[v].push_back(u);
        }
        from_source_ = parity_distances(source);
    }

    [[nodiscard]] std::array<std::int64_t, 2> minimum_lengths(int v) const
    {
        check_vertex(v);
        return from_source_[v];
    }

    [[nodiscard]] bool can_walk_to(int v, std::uint64_t length) const
    {
        const auto shortest = minimum_lengths(v)[length & 1U];
        if (length != 0 && adjacency_[source_].empty())
        {
            return false;
        }
        return shortest >= 0 && static_cast<std::uint64_t>(shortest) <= length;
    }

    [[nodiscard]] std::vector<int> vertices_on_walks(int target, std::uint64_t length) const
    {
        if (!can_walk_to(target, length))
        {
            return {};
        }
        if (length == 0)
        {
            return {source_};
        }
        const auto from_target = parity_distances(target);
        std::vector<int> result;
        for (std::size_t v = 0; v < adjacency_.size(); ++v)
        {
            for (unsigned parity = 0; parity < 2; ++parity)
            {
                const auto before = from_source_[v][parity];
                const auto after = from_target[v][parity ^ (length & 1U)];
                if (before >= 0 && static_cast<std::uint64_t>(before + after) <= length)
                {
                    result.push_back(static_cast<int>(v));
                    break;
                }
            }
        }
        return result;
    }

private:
    [[nodiscard]] std::vector<std::array<std::int64_t, 2>> parity_distances(int source) const
    {
        std::vector<std::array<std::int64_t, 2>> distance(adjacency_.size(), {-1, -1});
        std::vector<std::pair<int, unsigned>> pending{{source, 0}};
        distance[source][0] = 0;
        for (std::size_t i = 0; i < pending.size(); ++i)
        {
            const auto [u, parity] = pending[i];
            const auto next_parity = parity ^ 1U;
            for (const int v : adjacency_[u])
            {
                if (distance[v][next_parity] < 0)
                {
                    distance[v][next_parity] = distance[u][parity] + 1;
                    pending.emplace_back(v, next_parity);
                }
            }
        }
        return distance;
    }

    void check_vertex([[maybe_unused]] int v) const
    {
    }

    int source_;
    std::vector<std::vector<int>> adjacency_;
    std::vector<std::array<std::int64_t, 2>> from_source_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
