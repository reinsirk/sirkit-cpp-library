// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Shortest_Nonzero_Paths.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
template <typename Label, typename Combine, typename Invert>
void check(
    int n, const std::vector<sirkit::GroupPathEdge<Label>> &edges, int source, Label identity, Combine combine, Invert invert)
{
    using Distance = std::optional<std::uint64_t>;
    struct Arc
    {
        int target;
        std::uint32_t cost;
        Label label;
    };
    std::vector graph(n, std::vector<Arc>{});
    for (const auto &[u, v, cost, label] : edges)
    {
        graph[u].push_back({v, cost, label});
        graph[v].push_back({u, cost, invert(label)});
    }

    std::vector<Distance> expected(n);
    auto visit = [&](auto &self, int v, unsigned seen, std::uint64_t length, const Label &label) -> void
    {
        if (label != identity && (!expected[v] || length < *expected[v]))
        {
            expected[v] = length;
        }
        for (const auto &edge : graph[v])
        {
            if ((seen & (1u << edge.target)) == 0)
            {
                self(self, edge.target, seen | (1u << edge.target), length + edge.cost, combine(label, edge.label));
            }
        }
    };
    visit(visit, source, 1u << source, 0, identity);
    assert(sirkit::shortest_nonzero_paths(n, edges, source, identity, combine, invert) == expected);
}

using Permutation = std::array<int, 3>;
constexpr Permutation kIdentity{0, 1, 2};
using Edge = sirkit::GroupPathEdge<Permutation>;

Permutation compose(const Permutation &first, const Permutation &second)
{
    return {second[first[0]], second[first[1]], second[first[2]]};
}

Permutation perm_inverse(const Permutation &permutation)
{
    Permutation inverse;
    for (int x = 0; x < 3; ++x)
    {
        inverse[permutation[x]] = x;
    }
    return inverse;
}

template <typename Error, typename Action>
void check_error(Action action)
{
    bool caught = false;
    try
    {
        action();
    }
    catch (const Error &)
    {
        caught = true;
    }
    assert(caught);
}
} // namespace

int main()
{
    using XorEdge = sirkit::GroupPathEdge<unsigned>;
    constexpr auto kMaximum = std::numeric_limits<std::uint32_t>::max();

    check(1, std::vector<XorEdge>{}, 0, 0u, std::bit_xor<>{}, std::identity{});
    check(4, std::vector<XorEdge>{{0, 1, 1, 0}, {1, 2, 4, 1}, {3, 3, 0, 1}}, 0, 0u, std::bit_xor<>{}, std::identity{});
    check(4, std::vector<XorEdge>{{0, 1, 0, 0}, {1, 2, 0, 1}, {0, 2, 0, 0}, {1, 3, 3, 0}}, 0, 0u,
          std::bit_xor<>{}, std::identity{});
    check(4, std::vector<XorEdge>{{0, 1, kMaximum, 0}, {1, 2, kMaximum, 0}, {2, 3, kMaximum, 0},
                                  {0, 1, kMaximum, 1}, {2, 3, kMaximum, 1}},
          0, 0u, std::bit_xor<>{}, std::identity{});
    check(4, std::vector<XorEdge>{{0, 1, 8, 1}, {0, 2, 1, 0}, {2, 1, 2, 0}, {1, 3, 5, 0}, {2, 3, 9, 0},
                                  {0, 2, 6, 1}, {2, 2, 0, 1}},
          0, 0u, std::bit_xor<>{}, std::identity{});

    const std::array<Permutation, 6> permutations{{{0, 1, 2}, {0, 2, 1}, {1, 0, 2}, {1, 2, 0}, {2, 0, 1}, {2, 1, 0}}};
    assert(compose(permutations[2], permutations[3]) != compose(permutations[3], permutations[2]));
    check(4, std::vector<Edge>{{0, 1, 3, permutations[3]}, {1, 2, 9, permutations[2]},
                               {2, 0, 2, permutations[4]}, {1, 3, 8, kIdentity}},
          1, kIdentity, compose, perm_inverse);

    std::mt19937 random(40264062);
    for (int test = 0; test < 1600; ++test)
    {
        const int n = static_cast<int>(random() % 8) + 1;
        const int m = static_cast<int>(random() % 24);
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            const auto label = permutations[random() % permutations.size()];
            edges.push_back({static_cast<int>(random() % n), static_cast<int>(random() % n),
                             static_cast<std::uint32_t>(random() % 12), label});
        }
        const int source = static_cast<int>(random() % n);
        check(n, edges, source, kIdentity, compose, perm_inverse);
    }

#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[n, source] : std::vector<std::pair<int, int>>{{0, 0}, {2, -1}, {2, 2}, {-1, 0}})
    {
        check_error<std::out_of_range>(
            [&] { (void)sirkit::shortest_nonzero_paths(n, {}, source, kIdentity, compose, perm_inverse); });
    }
    for (const Edge edge : std::vector<Edge>{{-1, 0, 2, kIdentity}, {2, 0, 2, kIdentity}, {0, -1, 2, kIdentity}, {0, 2, 2, kIdentity}})
    {
        check_error<std::out_of_range>(
            [&] { (void)sirkit::shortest_nonzero_paths(2, std::span(&edge, 1), 0, kIdentity, compose, perm_inverse); });
    }
#endif
}
