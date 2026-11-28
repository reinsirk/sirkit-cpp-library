// SPDX-License-Identifier: CC0-1.0
// 生成版: Clang用。
// 生成元: Graph/Short_Simple_Path.cpp
// ドキュメント: https://github.com/reinsirk/sirkit-cpp-library/blob/main/Document/short_simple_path.md
// 参考（外積を用いた代表族）: https://arxiv.org/abs/1304.4626
//
// API一覧
// find_short_simple_path(graph,k,starts,ends): k頂点の単純パスを頂点列で1つ返す。存在しなければnullopt。
// 有向辺の隣接リスト。始点候補starts・終点候補endsは0始まり。1<=k<=8、頂点数は998244353未満。
// 乱数は使わない。候補列の重複を許す。多重辺・自己ループがあっても同じ頂点は再訪しない。
// n頂点m辺、D=binom(k,floor(k/2))としてO(k(n+m)D²(D+k))時間、O(m+nD(D+k))領域。始点と終点の候補の読み取り時間を別途加える。


// SIRKIT_NAMESPACE_V1: 公開APIはsirkit、既存のgeoはsirkit::geo、内部部品はsirkit::detail。
#include <algorithm>
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
#ifndef SIRKIT_SHORT_SIMPLE_PATH_DEFINED
#define SIRKIT_SHORT_SIMPLE_PATH_DEFINED

namespace detail
{
inline constexpr unsigned kShortPathLimit = 8;
inline constexpr std::uint32_t kShortPathMod = 998244353;

inline void ShortPathCheckParameters(std::size_t vertices, unsigned length)
{
    if (vertices >= kShortPathMod)
    {
        throw std::length_error("short simple path: too many vertices for distinct field labels");
    }
    if (length == 0 || length > kShortPathLimit)
    {
        throw std::invalid_argument("short simple path: number of vertices must be in [1,8]");
    }
}

inline std::size_t ShortPathVertex(int vertex, std::size_t vertices)
{
    if (vertex < 0 || static_cast<std::size_t>(vertex) >= vertices)
    {
        throw std::out_of_range("short simple path: vertex index out of range");
    }
    return static_cast<std::size_t>(vertex);
}

inline std::uint32_t ShortPathScalarProduct(std::uint32_t a, std::uint32_t b)
{
    return static_cast<std::uint32_t>(std::uint64_t{a} * b % kShortPathMod);
}

inline std::uint32_t ShortPathInverse(std::uint32_t value)
{
    std::uint32_t result = 1;
    for (auto exponent = kShortPathMod - 2; exponent != 0; exponent >>= 1)
    {
        if ((exponent & 1u) != 0)
        {
            result = ShortPathScalarProduct(result, value);
        }
        value = ShortPathScalarProduct(value, value);
    }
    return result;
}

inline std::uint32_t ShortPathDifference(std::uint32_t a, std::uint32_t b)
{
    return a >= b ? a - b : a + kShortPathMod - b;
}

using ShortPathVector = std::vector<std::uint32_t>;
using ShortPathLabels = std::array<std::uint32_t, kShortPathLimit>;

struct ShortPathRow
{
    std::array<int, kShortPathLimit> path{};
    ShortPathVector wedge;
};

struct ShortPathExteriorTerm
{
    std::size_t input;
    std::size_t output;
    unsigned power;
    bool negative;
};

inline std::vector<unsigned> ShortPathMasks(unsigned dimension, unsigned rank)
{
    std::vector<unsigned> masks;
    for (unsigned mask = 0; mask < (1u << dimension); ++mask)
    {
        if (static_cast<unsigned>(std::popcount(mask)) == rank)
        {
            masks.push_back(mask);
        }
    }
    return masks;
}

inline std::vector<ShortPathExteriorTerm> ShortPathExteriorTerms(unsigned dimension, std::span<const unsigned> previous,
                                                                 std::span<const unsigned> next)
{
    std::vector<ShortPathExteriorTerm> terms;
    std::array<std::size_t, 1u << kShortPathLimit> positions{};
    for (std::size_t i = 0; i < next.size(); ++i)
    {
        positions[next[i]] = i;
    }
    for (std::size_t index = 0; index < previous.size(); ++index)
    {
        const auto mask = previous[index];
        for (unsigned bit = 0; bit < dimension; ++bit)
        {
            if ((mask & (1u << bit)) == 0)
            {
                const bool negative = std::popcount(mask >> (bit + 1)) % 2 != 0;
                terms.push_back({index, positions[mask | (1u << bit)], bit, negative});
            }
        }
    }
    return terms;
}

inline ShortPathVector ShortPathExtend(std::span<const std::uint32_t> coefficients, const ShortPathLabels &label,
                                       std::span<const ShortPathExteriorTerm> terms, std::size_t size)
{
    ShortPathVector result(size);
    for (const auto &term : terms)
    {
        auto &value = result[term.output];
        const auto contribution = ShortPathScalarProduct(coefficients[term.input], label[term.power]);
        value = (term.negative ? value + kShortPathMod - contribution : value + contribution) % kShortPathMod;
    }
    return result;
}

inline bool ShortPathInsert(std::vector<ShortPathVector> &basis, ShortPathVector candidate)
{
    for (std::size_t lead = 0; lead < candidate.size(); ++lead)
    {
        const auto coefficient = candidate[lead];
        if (coefficient == 0)
        {
            continue;
        }
        const auto &row = basis[lead];
        if (row.empty())
        {
            const auto inverse = ShortPathInverse(coefficient);
            for (std::size_t j = lead; j < candidate.size(); ++j)
            {
                candidate[j] = ShortPathScalarProduct(candidate[j], inverse);
            }
            basis[lead] = std::move(candidate);
            return true;
        }
        for (std::size_t j = lead; j < candidate.size(); ++j)
        {
            candidate[j] = ShortPathDifference(candidate[j], ShortPathScalarProduct(coefficient, row[j]));
        }
    }
    return false;
}

inline bool ShortPathUsedVertex(const ShortPathRow &row, unsigned count, int vertex)
{
    const auto last = row.path.begin() + count;
    return std::find(row.path.begin(), last, vertex) != last;
}
} // namespace detail

[[nodiscard]] inline std::optional<std::vector<int>> find_short_simple_path(std::span<const std::vector<int>> graph,
                                                                            unsigned length,
                                                                            std::span<const int> starts,
                                                                            std::span<const int> ends)
{
    const auto n = graph.size();
    detail::ShortPathCheckParameters(n, length);
    std::vector<std::vector<std::size_t>> incoming(n);
    for (std::size_t u = 0; u < n; ++u)
    {
        for (const auto v : graph[u])
        {
            incoming[detail::ShortPathVertex(v, n)].push_back(u);
        }
    }
    std::vector<bool> can_start(n), can_end(n);
    for (const auto start : starts)
    {
        can_start[detail::ShortPathVertex(start, n)] = true;
    }
    for (const auto end : ends)
    {
        can_end[detail::ShortPathVertex(end, n)] = true;
    }
    if (length > n)
    {
        return std::nullopt;
    }
    if (length == 1)
    {
        for (std::size_t v = 0; v < n; ++v)
        {
            if (can_start[v] && can_end[v])
            {
                return std::vector<int>{static_cast<int>(v)};
            }
        }
        return std::nullopt;
    }

    std::vector<detail::ShortPathLabels> labels(n);
    std::vector<std::vector<detail::ShortPathRow>> layer(n);
    for (std::size_t v = 0; v < n; ++v)
    {
        auto &label = labels[v];
        label[0] = 1;
        for (unsigned power = 1; power < length; ++power)
        {
            label[power] = detail::ShortPathScalarProduct(label[power - 1], static_cast<std::uint32_t>(v + 1));
        }
        if (can_start[v])
        {
            auto &row = layer[v].emplace_back();
            row.path[0] = static_cast<int>(v);
            row.wedge.assign(label.begin(), label.begin() + length);
        }
    }
    auto masks = detail::ShortPathMasks(length, 1);
    for (unsigned count = 1; count + 1 < length; ++count)
    {
        const auto next_masks = detail::ShortPathMasks(length, count + 1);
        const auto terms = detail::ShortPathExteriorTerms(length, masks, next_masks);
        const auto dimension = next_masks.size();
        std::vector<std::vector<detail::ShortPathRow>> next(n);
        for (std::size_t v = 0; v < n; ++v)
        {
            const auto vertex = static_cast<int>(v);
            std::vector<detail::ShortPathVector> basis(dimension);
            for (const auto u : incoming[v])
            {
                for (const auto &path : layer[u])
                {
                    if (detail::ShortPathUsedVertex(path, count, vertex))
                    {
                        continue;
                    }
                    auto wedge = detail::ShortPathExtend(path.wedge, labels[v], terms, dimension);
                    if (detail::ShortPathInsert(basis, wedge))
                    {
                        auto &state = next[v].emplace_back(detail::ShortPathRow{path.path, std::move(wedge)});
                        state.path[count] = vertex;
                    }
                }
            }
        }
        // 同じ終点で外積を張る基底を残せば、どの後続の単純パスも見落とさない。
        masks = next_masks;
        layer.swap(next);
    }

    for (std::size_t v = 0; v < n; ++v)
    {
        if (!can_end[v])
        {
            continue;
        }
        const auto vertex = static_cast<int>(v);
        for (const auto u : incoming[v])
        {
            for (const auto &path : layer[u])
            {
                if (!detail::ShortPathUsedVertex(path, length - 1, vertex))
                {
                    std::vector<int> found(path.path.begin(), path.path.begin() + (length - 1));
                    found.push_back(vertex);
                    return found;
                }
            }
        }
    }
    return std::nullopt;
}

#endif

} // namespace sirkit
// SIRKIT_NAMESPACE_CLOSE_V1
// SIRKIT_NAMESPACE_END_V1
