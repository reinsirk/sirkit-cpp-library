// SPDX-License-Identifier: CC0-1.0
#include "Verify/current/common/fast_io.hpp"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <random>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;

struct Edge
{
    std::size_t u, v, color;
};

std::vector<std::vector<std::uint32_t>> SampleIntersectionMatrix(
    std::size_t rank, const std::vector<std::size_t> &offsets, const std::vector<Edge> &edges, std::mt19937_64 &random)
{
    std::vector<std::vector<std::uint32_t>> rows(offsets.back(), std::vector<std::uint32_t>(rank));
    std::uniform_int_distribution<std::uint32_t> coefficients(0, kMod - 1);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &[u, v, color] = edges[i];
        const auto point = i + 1;
        auto term = coefficients(random);
        for (std::size_t row = offsets[color]; row < offsets[color + 1]; ++row)
        {
            rows[row][u] = (rows[row][u] + term) % kMod;
            if (v < rank)
            {
                rows[row][v] = (rows[row][v] + kMod - term) % kMod;
            }
            term = static_cast<std::uint32_t>(std::uint64_t{term} * point % kMod);
        }
    }
    return rows;
}
} // namespace

int main()
{
    std::size_t n{}, m{}, colors{};
    verify_in >> n >> m >> colors;
    std::vector<std::size_t> offsets(colors + 1);
    for (std::size_t color = 0; color < colors; ++color)
    {
        std::size_t count{};
        verify_in >> count;
        offsets[color + 1] = offsets[color] + count;
    }
    std::vector<Edge> edges(m);
    for (auto &[u, v, color] : edges)
    {
        verify_in >> u >> v >> color;
        --u;
        --v;
        --color;
    }
    const auto rank = n - 1;
    std::vector<std::size_t> right(colors, offsets.back() + 1);
    std::mt19937_64 random(std::random_device{}());
    // 独立した3組の乱数で共通独立集合の最大サイズの下界を調べる。
    for (int trial = 0; trial < 3; ++trial)
    {
        const auto rows = SampleIntersectionMatrix(rank, offsets, edges, random);
        const auto ends = sirkit::interval_matrix_rank_ends(rows, rank);
        for (std::size_t color = 0; color < colors; ++color)
        {
            if (const auto end = ends[offsets[color]])
            {
                right[color] = std::min(right[color], end.value());
            }
        }
    }
    std::size_t answer = 0;
    for (const auto end : right)
    {
        const auto first = std::lower_bound(offsets.begin() + 1, offsets.end(), end);
        answer += static_cast<std::size_t>(offsets.end() - first);
    }
    verify_out << answer << '\n';
}
