// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Chromatic_Polynomial.cpp"

#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <random>
#include <span>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
constexpr std::uint32_t kMod = 998244353;
using Edge = std::pair<int, int>;

// 各辺で両端を同じ色にする場合を包除する。独立集合の分割は使わない。
std::vector<std::uint32_t> edge_inclusion_exclusion(int n, std::span<const Edge> edges)
{
    std::vector<int> signed_counts(n + 1);
    for (std::uint32_t selected = 0; selected < (std::uint32_t{1} << edges.size()); ++selected)
    {
        std::array<int, 8> representative{};
        std::iota(representative.begin(), representative.end(), 0);
        const auto root = [&representative](int vertex) {
            while (representative[vertex] != vertex)
            {
                vertex = representative[vertex];
            }
            return vertex;
        };
        int parts = n;
        for (auto mask = selected; mask != 0; mask &= mask - 1)
        {
            const auto &[u, v] = edges[std::countr_zero(mask)];
            const int a = root(u), b = root(v);
            if (a != b)
            {
                representative[a] = b;
                --parts;
            }
        }
        signed_counts[parts] += std::popcount(selected) % 2 == 0 ? 1 : -1;
    }
    std::vector<std::uint32_t> result;
    for (const auto coefficient : signed_counts)
    {
        result.push_back(static_cast<std::uint32_t>(std::int64_t{kMod} + coefficient) % kMod);
    }
    return result;
}

template<class Exception, class Operation>
void must_throw(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}
}

int main()
{
    using sirkit::chromatic_polynomial;
    assert((chromatic_polynomial(0, {}) == std::vector<std::uint32_t>{1}));
    assert((chromatic_polynomial(1, {}) == std::vector<std::uint32_t>{0, 1}));
    must_throw<std::length_error>([] { chromatic_polynomial(-1, {}); });
    must_throw<std::length_error>([] { chromatic_polynomial(21, {}); });
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &edge : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        must_throw<std::out_of_range>([&] { chromatic_polynomial(2, std::span(&edge, 1)); });
    }
#endif
    const std::array loop{Edge{1, 1}, Edge{0, 1}, Edge{1, 0}};
    assert(chromatic_polynomial(2, loop) == std::vector<std::uint32_t>(3));
    const std::array copies{Edge{2, 0}, Edge{0, 2}, Edge{1, 2}, Edge{2, 0}};
    assert(chromatic_polynomial(3, copies) == edge_inclusion_exclusion(3, copies));

    std::mt19937 random(20261128);
    for (int n = 1; n <= 7; ++n)
    {
        for (int query = 0; query < 64; ++query)
        {
            std::vector<Edge> edges;
            for (int v = 1; v < n; ++v)
            {
                for (int u = 0; u < v; ++u)
                {
                    if ((random() & 1) != 0 && edges.size() < 14)
                    {
                        edges.emplace_back(u, v);
                    }
                }
            }
            assert(chromatic_polynomial(n, edges) == edge_inclusion_exclusion(n, edges));
        }
    }
    const std::array single{Edge{0, 19}};
    std::vector<std::uint32_t> expected(21);
    expected[19] = kMod - 1;
    expected[20] = 1;
    assert(chromatic_polynomial(20, single) == expected);
}
