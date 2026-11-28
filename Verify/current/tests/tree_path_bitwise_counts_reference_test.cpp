// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Path_Bitwise_Counts.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::TreePathBitwiseEdge;

void check(int n, const std::vector<Edge> &edges)
{
    sirkit::TreePathBitwiseCounts expected;
    std::vector<std::vector<std::pair<int, std::uint64_t>>> graph(n);
    for (const auto &[u, v, weight] : edges)
    {
        graph[u].emplace_back(v, weight);
        graph[v].emplace_back(u, weight);
    }
    for (int source = 0; source < n; ++source)
    {
        auto traverse = [&](auto &self, int u, int p, std::uint64_t and_value, std::uint64_t or_value,
                            std::uint64_t xor_value) -> void
        {
            if (source < u)
            {
                for (int bit = 0; bit < 64; ++bit)
                {
                    expected.and_pairs[bit] += (and_value >> bit) & 1;
                    expected.or_pairs[bit] += (or_value >> bit) & 1;
                    expected.xor_pairs[bit] += (xor_value >> bit) & 1;
                }
            }
            for (const auto &[v, weight] : graph[u])
            {
                if (v != p)
                {
                    self(self, v, u, and_value & weight, or_value | weight, xor_value ^ weight);
                }
            }
        };
        traverse(traverse, source, -1, std::numeric_limits<std::uint64_t>::max(), 0, 0);
    }
    const auto actual = sirkit::tree_path_bitwise_counts(n, edges);
    assert(actual.and_pairs == expected.and_pairs);
    assert(actual.or_pairs == expected.or_pairs);
    assert(actual.xor_pairs == expected.xor_pairs);
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
    check(0, {});
    check(1, {});
    check(2, {{0, 1, 0}});
    check(6, {{0, 1, 0}, {1, 2, 2}, {1, 3, 3}, {4, 5, std::uint64_t{1} << 63}});
    check(4, {{0, 1, 0}, {1, 2, std::numeric_limits<std::uint64_t>::max()}, {2, 3, 1}});
    std::mt19937_64 random(39981333);
    for (int test = 0; test < 610; ++test)
    {
        const int n = static_cast<int>(random() % 11) + 1;
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            if (random() % 3 == 0)
            {
                continue;
            }
            const int u = static_cast<int>(random() % v);
            const std::uint64_t weight = test % 2 == 0 ? random() % 16 : random();
            edges.push_back(test % 3 == 0 ? Edge{u, v, weight} : Edge{v, u, weight});
        }
        check(n, edges);
    }
#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::length_error>([] { (void)sirkit::tree_path_bitwise_counts(-1, {}); });
    for (const Edge edge : std::vector<Edge>{{-1, 0, 1}, {2, 0, 1}, {0, -1, 1}, {0, 2, 1}})
    {
        check_error<std::out_of_range>([&] { (void)sirkit::tree_path_bitwise_counts(2, std::span(&edge, 1)); });
    }
    for (const auto &invalid : std::vector<std::vector<Edge>>{{{0, 0, 0}}, {{0, 1, 0}, {1, 0, 1}},
                                                           {{0, 1, 1}, {1, 2, 2}, {0, 2, 3}}})
    {
        check_error<std::invalid_argument>([&] { (void)sirkit::tree_path_bitwise_counts(3, invalid); });
    }
#endif
}
