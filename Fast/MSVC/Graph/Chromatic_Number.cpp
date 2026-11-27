// SPDX-License-Identifier: CC0-1.0
// 生成版: MSVC・速度優先（入力検査を除外）。 所有・解放の省略は各APIの前提を参照。
// 生成元: Graph/Chromatic_Number.cpp
// 参考（包除原理による彩色数）: https://eccc.weizmann.ac.il/report/2006/044/
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/chromatic_number.md
//
// API一覧
// chromatic_number(n, edges): 20頂点以下の無向グラフの最小色数を厳密に求める。
// minimum_vertex_coloring(n, edges): 最小色数color_countと、頂点ごとの色colorsを返す。
// 頂点・色は0始まり。多重辺を許し、自己辺があればどちらもnullopt。空グラフは0色。
// 部分集合DPと包除原理を使う。色数はO((1+n^3 log(n+1))2^n+m)時間、復元はO((1+n^4)2^n+m)時間。
// 補助領域はどちらもO(2^n)。

// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <array>
#include <bit>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

// SIRKIT_NAMESPACE_OPEN_V1
namespace sirkit {
#ifndef SIRKIT_CHROMATIC_NUMBER_DEFINED
#define SIRKIT_CHROMATIC_NUMBER_DEFINED

struct MinimumVertexColoring
{
    int color_count;
    std::vector<int> colors;
};

namespace detail
{
using ChromaticMask = std::uint32_t;
inline constexpr int kChromaticMaxVertices = 20;

struct ChromaticGraph
{
    int n;
    std::array<ChromaticMask, kChromaticMaxVertices> adjacency;
};

inline std::optional<ChromaticGraph> chromatic_graph(int n, std::span<const std::pair<int, int>> edges)
{
    if (n < 0 || n > kChromaticMaxVertices)
    {
        throw std::length_error("chromatic_number: vertex count must be in [0,20]");
    }
    ChromaticGraph graph{n, {}};
    bool has_loop = false;
    for (const auto &[u, v] : edges)
    {
        has_loop |= u == v;
        graph.adjacency[u] |= ChromaticMask{1} << v;
        graph.adjacency[v] |= ChromaticMask{1} << u;
    }
    if (has_loop)
    {
        return std::nullopt;
    }
    return graph;
}

struct ChromaticCoverTerm
{
    std::uint32_t base;
    int coefficient;
};

inline std::uint64_t chromatic_power64(std::uint64_t base, int exponent)
{
    std::uint64_t result = 1;
    for (; exponent != 0; exponent >>= 1)
    {
        if (exponent & 1)
        {
            result *= base;
        }
        base *= base;
    }
    return result;
}

inline bool chromatic_cover_nonzero_wide(std::span<const ChromaticCoverTerm> terms, int exponent)
{
    // i(S)^k <= 2^(n*k)。1桁32ビットで、n=k=20の積も収まる桁数を持つ。
    constexpr int kLimbBits = 32;
    constexpr int kLimbs = kChromaticMaxVertices * kChromaticMaxVertices / kLimbBits + 1;
    std::array<std::int64_t, kLimbs> sum{};
    for (const auto &[base, coefficient] : terms)
    {
        std::array<std::uint32_t, kLimbs> power{1};
        int length = 1;
        for (int i = 0; i < exponent; ++i)
        {
            std::uint64_t carry = 0;
            for (int j = 0; j < length; ++j)
            {
                const auto product = std::uint64_t{power[j]} * base + carry;
                power[j] = static_cast<std::uint32_t>(product);
                carry = product >> kLimbBits;
            }
            if (carry != 0)
            {
                power[length++] = static_cast<std::uint32_t>(carry);
            }
        }
        for (int j = 0; j < length; ++j)
        {
            sum[j] += std::int64_t{power[j]} * coefficient;
        }
    }
    // 係数の絶対値の総和は2^n以下。各桁の和と繰り上がりはint64_tに収まる。
    std::int64_t carry = 0;
    for (const auto limb : sum)
    {
        const auto value = limb + carry;
        if (static_cast<std::uint32_t>(value) != 0)
        {
            return true;
        }
        carry = value / (std::int64_t{1} << kLimbBits);
    }
    // 被覆数自体が全桁に収まるので、全桁0なら0。
    return false;
}

class ChromaticCover
{
    std::vector<ChromaticCoverTerm> terms_;
    int upper_bits_;

public:
    explicit ChromaticCover(const ChromaticGraph &graph)
    {
        const ChromaticMask limit = ChromaticMask{1} << graph.n;
        std::vector<std::uint32_t> counts(limit);
        counts[0] = 1;
        for (ChromaticMask mask = 1; mask < limit; ++mask)
        {
            const auto v = std::countr_zero(mask);
            const auto rest = mask & (mask - 1);
            counts[mask] = counts[rest] + counts[rest & ~graph.adjacency[v]];
        }
        upper_bits_ = std::bit_width(counts.back() - 1);

        std::vector<int> coefficients(counts.back() + 1);
        for (ChromaticMask mask = 0; mask < limit; ++mask)
        {
            const bool negative = (graph.n - std::popcount(mask)) & 1;
            coefficients[counts[mask]] += negative ? -1 : 1;
        }
        for (std::uint32_t base = 1; base < coefficients.size(); ++base)
        {
            if (coefficients[base] != 0)
            {
                terms_.push_back({base, coefficients[base]});
            }
        }
    }

    bool can_color(int k) const
    {
        // 独立集合をk個選んで全頂点を覆う方法を数える。重複して覆われた頂点は任意の1色に割り当てられる。
        if (upper_bits_ * k > 64)
        {
            return chromatic_cover_nonzero_wide(terms_, k);
        }
        std::uint64_t count = 0;
        for (const auto &[base, coefficient] : terms_)
        {
            count += chromatic_power64(base, k) * static_cast<std::uint64_t>(coefficient);
        }
        // k>0、n>0では全色空集合の選び方を除くため、被覆数は2^64未満。
        return count != 0;
    }
};

inline int minimum_chromatic_count(const ChromaticGraph &graph)
{
    if (graph.n <= 1)
    {
        return graph.n;
    }
    const ChromaticCover cover(graph);
    int lower = 0, upper = graph.n;
    while (upper - lower > 1)
    {
        const int k = lower + (upper - lower) / 2;
        if (cover.can_color(k))
        {
            upper = k;
        }
        else
        {
            lower = k;
        }
    }
    return upper;
}

inline ChromaticGraph chromatic_merge_last(ChromaticGraph graph, int u)
{
    const int v = --graph.n;
    const auto neighbors = graph.adjacency[v];
    const auto removed = ChromaticMask{1} << v;
    graph.adjacency[u] |= neighbors;
    for (int i = 0; i < graph.n; ++i)
    {
        graph.adjacency[i] &= ~removed;
        if ((neighbors >> i) & 1)
        {
            graph.adjacency[i] |= ChromaticMask{1} << u;
        }
    }
    return graph;
}
} // namespace detail

inline std::optional<int> chromatic_number(int n, std::span<const std::pair<int, int>> edges)
{
    const auto graph = detail::chromatic_graph(n, edges);
    if (!graph)
    {
        return std::nullopt;
    }
    return detail::minimum_chromatic_count(*graph);
}

inline std::optional<MinimumVertexColoring> minimum_vertex_coloring(
    int n, std::span<const std::pair<int, int>> edges)
{
    auto graph = detail::chromatic_graph(n, edges);
    if (!graph)
    {
        return std::nullopt;
    }
    int colors = detail::minimum_chromatic_count(*graph);
    MinimumVertexColoring result{colors, std::vector<int>(n)};
    using Mask = detail::ChromaticMask;
    std::array<Mask, detail::kChromaticMaxVertices> groups{};
    for (int i = 0; i < n; ++i)
    {
        groups[i] = Mask{1} << i;
    }

    while (graph->n != 0)
    {
        const int last = graph->n - 1;
        const Mask candidates = ((Mask{1} << last) - 1) & ~graph->adjacency[last];
        bool merged = false;
        for (Mask choices = candidates; choices != 0; choices &= choices - 1)
        {
            const auto u = std::countr_zero(choices);
            auto smaller = detail::chromatic_merge_last(*graph, u);
            if (smaller.n == colors || detail::ChromaticCover(smaller).can_color(colors))
            {
                groups[u] |= groups[last];
                *graph = smaller;
                merged = true;
                break;
            }
        }
        if (merged)
        {
            continue;
        }

        // 最後の頂点と同色にできる頂点がないので、残りには1色少なく割り当てられる。
        // 取り除いた頂点のビットは、有効頂点だけのマスクで次回以降除かれる。
        --graph->n;
        --colors;
        for (Mask members = groups[last]; members != 0; members &= members - 1)
        {
            result.colors[std::countr_zero(members)] = colors;
        }
    }
    return result;
}

#endif
} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
