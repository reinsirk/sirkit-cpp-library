// SPDX-License-Identifier: CC0-1.0
// 生成版: GCC用。
// 生成元: Graph/Degree_Sequence.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/degree_sequence.md
// 参考（Erdős–GallaiとHavel–Hakimi）: https://info.atcoder.jp/entry/algorithm_lectures/degree_sequence
//
// API一覧
// is_graphical_degree_sequence(degrees): 単純無向グラフの次数列になり得るかをO(n+1)で判定。
// realize_degree_sequence(degrees): 指定次数の単純無向グラフを0始まりの辺リストで返す。不能ならnullopt。
// 構築は辺数mについてO(n+m+1)時間、結果以外O(n+1)領域。空の次数列は空グラフ。
// 負の次数やn以上の次数は不能。頂点数はintの範囲内。容量超過はlength_error。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
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
#ifndef SIRKIT_DEGREE_SEQUENCE_DEFINED
#define SIRKIT_DEGREE_SEQUENCE_DEFINED

namespace detail
{
inline int DegreeSequenceVertexCount(std::size_t size)
{
    if (size > static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max()))
    {
        throw std::length_error("degree sequence: too many vertices");
    }
    return static_cast<int>(size);
}

inline std::size_t DegreeSequenceEdgeCapacity(std::uint64_t edges)
{
    if (edges > std::vector<std::pair<int, int>>{}.max_size())
    {
        throw std::length_error("degree sequence: too many edges");
    }
    return static_cast<std::size_t>(edges);
}

inline std::optional<std::uint64_t> GraphicalDegreeSum(std::span<const int> degrees)
{
    const auto n = DegreeSequenceVertexCount(degrees.size());
    std::vector<std::size_t> count(degrees.size());
    std::uint64_t total = 0;
    for (const int degree : degrees)
    {
        if (degree < 0 || degree >= n)
        {
            return std::nullopt;
        }
        ++count[static_cast<std::size_t>(degree)];
        total += static_cast<std::uint64_t>(degree);
    }
    if ((total & 1u) != 0)
    {
        return std::nullopt;
    }

    std::vector<std::size_t> sorted(degrees.size());
    std::vector<std::uint64_t> prefix(degrees.size() + 1);
    std::size_t head = 0;
    for (auto next = count.size(); next != 0; --next)
    {
        const auto degree = next - 1;
        for (std::size_t i = 0; i < count[degree]; ++i)
        {
            sorted[head] = degree;
            prefix[head + 1] = prefix[head] + degree;
            ++head;
        }
    }
    auto cutoff = sorted.size();
    for (std::size_t first = 1; first <= sorted.size(); ++first)
    {
        cutoff = std::max(first, cutoff);
        while (cutoff > first && sorted[cutoff - 1] < first)
        {
            --cutoff;
        }
        const auto available = std::uint64_t{first} * (cutoff - 1) + (total - prefix[cutoff]);
        if (prefix[first] > available)
        {
            return std::nullopt;
        }
    }
    return total;
}
} // namespace detail

[[nodiscard]] inline bool is_graphical_degree_sequence(std::span<const int> degrees)
{
    return detail::GraphicalDegreeSum(degrees).has_value();
}

[[nodiscard]] inline std::optional<std::vector<std::pair<int, int>>> realize_degree_sequence(std::span<const int> degrees)
{
    const auto total = detail::GraphicalDegreeSum(degrees);
    if (!total)
    {
        return std::nullopt;
    }
    std::vector<std::pair<int, int>> edges;
    edges.reserve(detail::DegreeSequenceEdgeCapacity(total.value() / 2));
    std::vector<int> head(degrees.size(), -1), next(degrees.size(), -1);
    const auto insert = [&](int vertex, int degree)
    {
        auto &front = head[static_cast<std::size_t>(degree)];
        next[static_cast<std::size_t>(vertex)] = front;
        front = vertex;
    };
    const auto pop = [&](int degree)
    {
        auto &front = head[static_cast<std::size_t>(degree)];
        const int vertex = front;
        front = next[static_cast<std::size_t>(vertex)];
        return vertex;
    };
    int maximum = 0;
    for (std::size_t vertex = 0; vertex < degrees.size(); ++vertex)
    {
        maximum = std::max(maximum, degrees[vertex]);
        insert(static_cast<int>(vertex), degrees[vertex]);
    }
    std::vector<std::pair<int, int>> removed;
    while (maximum > 0)
    {
        if (head[static_cast<std::size_t>(maximum)] == -1)
        {
            --maximum;
            continue;
        }
        const int center = pop(maximum);
        removed.clear(); // 近傍を重複して選ばないよう、次数の更新は選び終えてから行う。
        int remaining = maximum;
        for (int i = 0; i < maximum; ++i)
        {
            while (head[static_cast<std::size_t>(remaining)] == -1)
            {
                --remaining;
            }
            const int vertex = pop(remaining);
            edges.emplace_back(center, vertex);
            removed.emplace_back(vertex, remaining - 1);
        }
        for (const auto &[vertex, degree] : removed)
        {
            insert(vertex, degree);
        }
    }
    return edges;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
