// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC用。
// 生成元: Graph/Minimum_Spanning_Tree.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/minimum_spanning_tree.md
//
// API一覧
// SpanningTreeEdge: 無向辺{from,to,weight}。weightは符号付き64ビット。
// minimum_spanning_forest(n,edges): Kruskal法で選んだ辺の、入力列での添字を返す。
// 非連結なら各連結成分の最小全域木を求める。自己ループ、多重辺、負の重みも可。
// 出力は重みの昇順で、同じ重みでは入力順。頂点と辺の番号は0始まり。O(N+M log(M+1))時間・O(N+M)領域。
// 重み差と辺番号を64ビットに詰められる大きな入力は基数ソートで整列する。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <numeric>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_MINIMUM_SPANNING_TREE_DEFINED
#define SIRKIT_MINIMUM_SPANNING_TREE_DEFINED

struct SpanningTreeEdge
{
    int from;
    int to;
    std::int64_t weight;
};

namespace detail
{
inline void SortSpanningEdgeWeights(std::vector<std::uint64_t> &keys, int id_bits, int weight_bits)
{
    constexpr unsigned kDigitBits = 8;
    constexpr std::size_t kBuckets = 1U << kDigitBits;
    constexpr auto kDigitMask = kBuckets - 1;
    std::vector<std::uint64_t> temporary(keys.size());
    for (int offset = 0; offset < weight_bits; offset += kDigitBits)
    {
        const int shift = offset + id_bits;
        std::array<std::size_t, kBuckets> count{};
        for (auto key : keys)
        {
            ++count[(key >> shift) & kDigitMask];
        }
        std::size_t seen = 0;
        for (auto &bucket : count)
        {
            const auto frequency = bucket;
            bucket = seen;
            seen += frequency;
        }
        for (auto key : keys)
        {
            temporary[count[(key >> shift) & kDigitMask]++] = key;
        }
        keys.swap(temporary);
    }
}
} // namespace detail

inline std::vector<std::size_t> minimum_spanning_forest(int n, std::span<const SpanningTreeEdge> edges)
{
#ifndef SKIP_RUNTIME_CHECKS
    if (n < 0)
    {
        throw std::invalid_argument("minimum_spanning_forest: negative vertex count");
    }
#endif
    if (edges.empty())
    {
        return {};
    }

    constexpr std::size_t kRadixThreshold = 1024;
    const bool try_radix = edges.size() >= kRadixThreshold;
    auto minimum = edges.front().weight, maximum = minimum;
#ifdef SKIP_RUNTIME_CHECKS
    if (try_radix)
#endif
    {
        for (const auto &[from, to, weight] : edges)
        {
#ifndef SKIP_RUNTIME_CHECKS
            if (from < 0 || from >= n || to < 0 || to >= n)
            {
                throw std::out_of_range("minimum_spanning_forest: edge endpoint");
            }
#endif
            minimum = std::min(minimum, weight);
            maximum = std::max(maximum, weight);
        }
    }

    std::vector<std::size_t> result;
    if (n < 2)
    {
        return result;
    }
    std::vector<std::uint64_t> order(edges.size());
    auto id_mask = std::numeric_limits<std::uint64_t>::max();
    const auto unsigned_min = static_cast<std::uint64_t>(minimum);
    const auto weight_range = static_cast<std::uint64_t>(maximum) - unsigned_min;
    const int id_bits = std::bit_width(edges.size() - 1);
    const int weight_bits = std::bit_width(weight_range);
    if (try_radix && weight_bits <= 64 - id_bits)
    {
        if (weight_range == 0)
        {
            std::iota(order.begin(), order.end(), std::uint64_t{0});
        }
        else
        {
            // 下位ビットに辺番号を載せ、上位の重みだけを安定に並べる。
            id_mask = (std::uint64_t{1} << id_bits) - 1;
            for (std::size_t i = 0; i < order.size(); ++i)
            {
                order[i] = ((static_cast<std::uint64_t>(edges[i].weight) - unsigned_min) << id_bits) | i;
            }
            detail::SortSpanningEdgeWeights(order, id_bits, weight_bits);
        }
    }
    else
    {
        std::iota(order.begin(), order.end(), std::uint64_t{0});
        std::sort(order.begin(), order.end(), [&](auto a, auto b)
                  {
                      const auto left = edges[static_cast<std::size_t>(a)].weight;
                      const auto right = edges[static_cast<std::size_t>(b)].weight;
                      return left == right ? a < b : left < right;
                  });
    }

    std::vector<int> components(n, -1);
    const auto root = [&components](int vertex)
    {
        while (components[vertex] >= 0)
        {
            const int parent = components[vertex];
            if (components[parent] < 0)
            {
                return parent;
            }
            vertex = components[vertex] = components[parent];
        }
        return vertex;
    };
    const auto limit = static_cast<std::size_t>(n - 1);
    result.reserve(std::min(limit, edges.size()));
    for (auto key : order)
    {
        const auto id = static_cast<std::size_t>(key & id_mask);
        const auto &edge = edges[id];
        int a = root(edge.from), b = root(edge.to);
        if (a == b)
        {
            continue;
        }
        if (components[a] > components[b])
        {
            std::swap(a, b);
        }
        components[a] += components[b];
        components[b] = a;
        result.push_back(id);
        if (result.size() == limit)
        {
            break;
        }
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
