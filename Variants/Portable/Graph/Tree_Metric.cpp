// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通用。
// 生成元: Graph/Tree_Metric.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/tree_metric.md
//
// API一覧
// reconstruct_tree_metric(n,distance):
//   頂点がちょうど0..n-1の正重み無向木を全頂点間距離から復元する。不可能ならnullopt。
//   distance(u,v)は符号付き・符号なし64bit以下の整数。対角は0、非対角は正で対称。
//   結果はTreeMetricEdge{from,to,weight}の列。n=0,1で整合する場合は空列。
//   distanceの呼出しがO(1)ならO(n^2)時間、O(n)追加領域。
//   復元木の各辺は、その切断をまたぐ最小の距離。密なPrimで候補を立てて全距離を照合する。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_TREE_METRIC_DEFINED
#define SIRKIT_TREE_METRIC_DEFINED

struct TreeMetricEdge
{
    int from;
    int to;
    std::uint64_t weight;
};

template <typename Distance>
[[nodiscard]] std::optional<std::vector<TreeMetricEdge>> reconstruct_tree_metric(int n, Distance distance)
{
    using Weight = std::remove_cvref_t<decltype(distance(0, 0))>;
    static_assert(std::is_integral_v<Weight> && sizeof(Weight) <= sizeof(std::uint64_t));
    if (n < 0)
    {
        throw std::length_error("reconstruct_tree_metric: negative vertex count");
    }
    auto read_distance = [&](int u, int v) -> std::optional<std::uint64_t>
    {
        const auto d = distance(u, v);
        if constexpr (std::is_signed_v<Weight>)
        {
            if (d < 0)
            {
                return std::nullopt;
            }
        }
        return static_cast<std::uint64_t>(d);
    };

    std::vector<TreeMetricEdge> result;
    if (n == 0)
    {
        return result;
    }
    result.reserve(n - 1);
    std::vector<std::vector<std::pair<int, std::uint64_t>>> graph(n);
    std::vector<std::uint64_t> closest(n);
    std::vector<int> parent(n, -1);
    std::vector<bool> used(n);
    int best = 0;
    for (int i = 0; i < n; ++i)
    {
        if (i != 0)
        {
            const int p = parent[best];
            result.push_back({p, best, closest[best]});
            graph[p].emplace_back(best, closest[best]);
            graph[best].emplace_back(p, closest[best]);
        }
        used[best] = true;
        int next = -1;
        for (int v = 0; v < n; ++v)
        {
            if (used[v])
            {
                continue;
            }
            const auto d = read_distance(best, v);
            if (!d || *d == 0)
            {
                return std::nullopt;
            }
            if (parent[v] == -1 || *d < closest[v])
            {
                closest[v] = *d;
                parent[v] = best;
            }
            if (next == -1 || closest[v] < closest[next])
            {
                next = v;
            }
        }
        best = next;
    }

    std::vector<std::tuple<int, int, std::uint64_t>> pending;
    for (int source = 0; source < n; ++source)
    {
        pending.emplace_back(source, -1, 0);
        while (!pending.empty())
        {
            const auto [v, previous, total] = pending.back();
            pending.pop_back();
            if (read_distance(source, v) != total)
            {
                return std::nullopt;
            }
            for (const auto &[u, weight] : graph[v])
            {
                if (u != previous)
                {
                    if (weight > std::numeric_limits<std::uint64_t>::max() - total)
                    {
                        return std::nullopt;
                    }
                    pending.emplace_back(u, v, total + weight);
                }
            }
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
