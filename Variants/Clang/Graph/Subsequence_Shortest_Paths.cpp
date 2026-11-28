// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Subsequence_Shortest_Paths.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/subsequence_shortest_paths.md
//
// API一覧
// SubsequenceShortestPaths<T>(n, edges, schedule, source): 0 始まり。辺{from,to,weight}と辺番号列を渡す。
//   辺番号列の部分列に沿って移動する、始点からの全最短距離を求める。負の辺、再訪、同じ辺の再登場も可。
//   T は bool 以外の整数型。到達可能な頂点から試みる緩和の重み和が T に収まること。
//   O(n+m+q) 時間・O(n+q) 領域。m は辺数で q は辺番号列の長さ。n は正。
// distance_to(v)/distances(): 距離の optional / 頂点番号順の const span。未到達は nullopt。O(1)。
// path_to(v): 最短経路の {occurrence,edge_id} 列。前者は schedule 内の位置。未到達は nullopt。
//   長さ L に対して O(L+1) 時間・領域。始点から移動しない解は空列。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <concepts>
#include <cstddef>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_SUBSEQUENCE_SHORTEST_PATHS_DEFINED
#define SIRKIT_SUBSEQUENCE_SHORTEST_PATHS_DEFINED

template <class T>
struct SubsequencePathEdge
{
    int from, to;
    T weight;
};

struct SubsequencePathStep
{
    std::size_t occurrence;
    int edge_id;
};

template <std::integral T>
    requires(!std::same_as<T, bool>)
class SubsequenceShortestPaths
{
public:
    SubsequenceShortestPaths(int n, std::span<const SubsequencePathEdge<T>> edges,
                             std::span<const int> schedule, int source)
    {
        if (n <= 0)
        {
            throw std::length_error("SubsequenceShortestPaths: vertex count must be positive");
        }
        if (source < 0 || source >= n)
        {
            throw std::out_of_range("SubsequenceShortestPaths: source");
        }
        for (const auto &[u, v, weight] : edges)
        {
            if (u < 0 || u >= n || v < 0 || v >= n)
            {
                throw std::out_of_range("SubsequenceShortestPaths: edge endpoint");
            }
        }

        costs_.resize(n);
        tails_.assign(n, kNoRecord);
        records_.reserve(schedule.size());
        costs_[source] = T{0};
        for (std::size_t at = 0; at < schedule.size(); ++at)
        {
            const int id = schedule[at];
            if (id < 0 || static_cast<std::size_t>(id) >= edges.size())
            {
                throw std::out_of_range("SubsequenceShortestPaths: schedule edge");
            }
            const auto [u, v, weight] = edges[id];
            if (!costs_[u])
            {
                continue;
            }
            const T new_cost = checked_add(*costs_[u], weight);
            if (!costs_[v] || new_cost < *costs_[v])
            {
                records_.push_back({tails_[u], {at, id}});
                tails_[v] = records_.size() - 1;
                costs_[v] = new_cost;
            }
        }
    }

    [[nodiscard]] std::span<const std::optional<T>> distances() const
    {
        return costs_;
    }

    [[nodiscard]] std::optional<T> distance_to(int v) const
    {
        check_vertex(v);
        return costs_[v];
    }

    [[nodiscard]] std::optional<std::vector<SubsequencePathStep>> path_to(int v) const
    {
        check_vertex(v);
        if (!costs_[v])
        {
            return std::nullopt;
        }
        std::vector<SubsequencePathStep> result;
        for (auto i = tails_[v]; i != kNoRecord; i = records_[i].previous)
        {
            result.push_back(records_[i].step);
        }
        std::reverse(result.begin(), result.end());
        return result;
    }

private:
    static T checked_add(T base, T weight)
    {
        if (weight > 0 && base > std::numeric_limits<T>::max() - weight)
        {
            throw std::overflow_error("SubsequenceShortestPaths: weight sum above range");
        }
        if constexpr (std::signed_integral<T>)
        {
            if (weight < 0 && base < std::numeric_limits<T>::lowest() - weight)
            {
                throw std::overflow_error("SubsequenceShortestPaths: weight sum below range");
            }
        }
        return static_cast<T>(base + weight);
    }

    void check_vertex(int v) const
    {
        if (v < 0 || static_cast<std::size_t>(v) >= costs_.size())
        {
            throw std::out_of_range("SubsequenceShortestPaths: query vertex");
        }
    }

    static constexpr std::size_t kNoRecord = std::numeric_limits<std::size_t>::max();
    struct Record
    {
        std::size_t previous;
        SubsequencePathStep step;
    };
    std::vector<std::optional<T>> costs_;
    std::vector<std::size_t> tails_;
    std::vector<Record> records_;
};

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
