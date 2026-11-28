// SPDX-License-Identifier: CC0-1.0
// 生成版: コンパイラ共通・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Unit_Step_Forest_Labels.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/unit_step_forest_labels.md
//
// API一覧
// IntegerLabelInterval{lower,upper}: int64_t の閉区間。既定値は型の全範囲。
// unit_step_forest_labels(n,edges,allowed): 無向森の各頂点に allowed[v] 内の整数を割り当てる。
//   各辺の両端の差の絶対値は1。頂点は0始まりで、各無向辺を pair<int,int> で1回渡す。
//   成功時は頂点順のラベル列、解なしならnullopt。O(n) 時間・領域。
//   部分木で実現可能な値は、偶数と奇数で分けるとそれぞれ2刻みの区間になる。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>


// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_UNIT_STEP_FOREST_LABELS_DEFINED
#define SIRKIT_UNIT_STEP_FOREST_LABELS_DEFINED

struct IntegerLabelInterval
{
    std::int64_t lower = std::numeric_limits<std::int64_t>::min();
    std::int64_t upper = std::numeric_limits<std::int64_t>::max();
};

inline std::optional<std::vector<std::int64_t>> unit_step_forest_labels(
    int n, std::span<const std::pair<int, int>> edges, std::span<const IntegerLabelInterval> allowed)
{
    if (n < 0)
    {
        throw std::length_error("unit_step_forest_labels: negative vertex count");
    }
    std::vector<std::vector<int>> graph(n);
    for (const auto &[u, v] : edges)
    {
        graph[u].push_back(v);
        graph[v].push_back(u);
    }

    std::vector<int> parent(n, -2), order;
    order.reserve(n);
    for (int root = 0; root < n; ++root)
    {
        if (parent[root] != -2)
        {
            continue;
        }
        std::size_t index = order.size();
        parent[root] = -1;
        order.push_back(root);
        for (; index < order.size(); ++index)
        {
            const int v = order[index];
            for (int next : graph[v])
            {
                if (next == parent[v])
                {
                    continue;
                }
                parent[next] = v;
                order.push_back(next);
            }
        }
    }

    constexpr auto kMin = std::numeric_limits<std::int64_t>::min();
    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    using Bounds = std::optional<IntegerLabelInterval>;
    std::vector<std::array<Bounds, 2>> feasible(n);
    for (int v = 0; v < n; ++v)
    {
        for (int parity = 0; parity < 2; ++parity)
        {
            auto [lower, upper] = allowed[v];
            if ((lower & 1) != parity)
            {
                if (lower == kMax)
                {
                    continue;
                }
                ++lower;
            }
            if ((upper & 1) != parity)
            {
                if (upper == kMin)
                {
                    continue;
                }
                --upper;
            }
            if (lower <= upper)
            {
                feasible[v][parity] = IntegerLabelInterval{lower, upper};
            }
        }
    }

    for (auto it = order.rbegin(); it != order.rend(); ++it)
    {
        const int v = *it;
        const int p = parent[v];
        if (p == -1)
        {
            continue;
        }
        for (int parity = 0; parity < 2; ++parity)
        {
            auto &prior = feasible[p][parity];
            const auto &child = feasible[v][1 - parity];
            if (prior && child)
            {
                const auto lower = child->lower == kMin ? kMin : child->lower - 1;
                const auto upper = child->upper == kMax ? kMax : child->upper + 1;
                prior->lower = std::max(prior->lower, lower);
                prior->upper = std::min(prior->upper, upper);
                if (prior->lower > prior->upper)
                {
                    prior.reset();
                }
            }
            else
            {
                prior.reset();
            }
        }
    }

    std::vector<std::int64_t> result(n);
    for (int v : order)
    {
        const int p = parent[v];
        if (p == -1)
        {
            const int parity = feasible[v][0] ? 0 : 1;
            const auto &bounds = feasible[v][parity];
            if (!bounds)
            {
                return std::nullopt;
            }
            result[v] = std::clamp<std::int64_t>(parity, bounds->lower, bounds->upper);
        }
        else
        {
            const auto base = result[p];
            const auto &bounds = *feasible[v][1 - (base & 1)];
            if (base < kMax && base + 1 <= bounds.upper)
            {
                result[v] = base + 1;
            }
            else
            {
                result[v] = base - 1;
            }
        }
    }
    return result;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
