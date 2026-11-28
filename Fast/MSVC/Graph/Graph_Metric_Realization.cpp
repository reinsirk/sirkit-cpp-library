// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Graph_Metric_Realization.cpp
// 参考（三角不等式と不要辺）: https://atcoder.jp/img/arc083/editorial.pdf
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/graph_metric_realization.md
//
// API一覧
// realize_graph_metric(n,distance,directed=false): 全頂点間の最短距離から、辺重みの合計が最小のグラフを復元。
//   distance(u,v)は64bit以下の整数かoptional<uint64_t>。頂点番号は0始まり、対角は0、有限の非対角は正。
//   nulloptは到達不能。無向の場合は対称。距離表に矛盾があれば戻り値はnullopt。
// GraphMetricRealization{edges,total_weight}: 復元辺MetricGraphEdge{from,to,weight}の列と重みの合計。
//   合計だけがuint64_tの上限を超える場合、edgesを返しtotal_weightはnullopt。
//   三角不等式が等号となる辺を除く。distanceがO(1)ならO(n^3)時間・O(n^2)領域。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>
#include <type_traits>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_GRAPH_METRIC_REALIZATION_DEFINED
#define SIRKIT_GRAPH_METRIC_REALIZATION_DEFINED

struct MetricGraphEdge
{
    int from;
    int to;
    std::uint64_t weight;
};

struct GraphMetricRealization
{
    std::vector<MetricGraphEdge> edges;
    std::optional<std::uint64_t> total_weight = 0;
};

namespace detail
{
inline std::size_t graph_metric_area(std::size_t n, std::size_t max_area)
{
    if (n != 0 && n > max_area / n)
    {
        throw std::length_error("realize_graph_metric: distance matrix too large");
    }
    return n * n;
}
} // namespace detail

template <typename Distance>
[[nodiscard]] std::optional<GraphMetricRealization> realize_graph_metric(int n, Distance distance, bool directed = false)
{
    using Value = std::remove_cvref_t<decltype(distance(0, 0))>;
    using Entry = std::optional<std::uint64_t>;
    static_assert((std::is_integral_v<Value> && sizeof(Value) <= sizeof(std::uint64_t)) ||
                  std::is_same_v<Value, Entry>);
    if (n < 0)
    {
        throw std::length_error("realize_graph_metric: negative vertex count");
    }
    std::vector<Entry> matrix(detail::graph_metric_area(n, std::vector<Entry>{}.max_size()));
    auto at = [&](int u, int v) -> Entry &
    {
        return matrix[static_cast<std::size_t>(u) * n + v];
    };
    for (int u = 0; u < n; ++u)
    {
        for (int v = 0; v < n; ++v)
        {
            const auto raw = distance(u, v);
            if constexpr (std::is_integral_v<Value> && std::is_signed_v<Value>)
            {
                if (raw < 0)
                {
                    return std::nullopt;
                }
            }
            const Entry d{raw};
            if (u == v)
            {
                if (d != std::uint64_t{0})
                {
                    return std::nullopt;
                }
            }
            else if (d == std::uint64_t{0})
            {
                return std::nullopt;
            }
            if (!directed && v < u && d != at(v, u))
            {
                return std::nullopt;
            }
            at(u, v) = d;
        }
    }

    GraphMetricRealization result;
    for (int u = 0; u < n; ++u)
    {
        for (int v = directed ? 0 : u + 1; v < n; ++v)
        {
            if (u == v)
            {
                continue;
            }
            const auto &d = at(u, v);
            bool redundant = false;
            for (int middle = 0; middle < n; ++middle)
            {
                if (middle == u || middle == v)
                {
                    continue;
                }
                const auto &before = at(u, middle);
                const auto &after = at(middle, v);
                if (!before || !after)
                {
                    continue;
                }
                if (!d)
                {
                    return std::nullopt;
                }
                if (*before >= *d)
                {
                    continue;
                }
                const auto rest = *d - *before;
                if (*after < rest)
                {
                    return std::nullopt;
                }
                redundant |= *after == rest;
            }
            if (d && !redundant)
            {
                result.edges.push_back({u, v, *d});
                if (result.total_weight)
                {
                    if (*d > std::numeric_limits<std::uint64_t>::max() - *result.total_weight)
                    {
                        result.total_weight.reset();
                    }
                    else
                    {
                        *result.total_weight += *d;
                    }
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
