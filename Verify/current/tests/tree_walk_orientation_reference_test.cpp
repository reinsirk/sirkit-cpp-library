// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Walk_Orientation.cpp"

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Pair = std::pair<int, int>;

void compare(int n, const std::vector<Pair> &edges, const std::vector<Pair> &walks)
{
    const auto result = sirkit::orient_tree_walks(n, edges, walks);
    assert(result.reversed.size() == walks.size());
    std::vector<std::vector<std::pair<int, std::size_t>>> tree(n);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto [u, v] = edges[i];
        tree[u].emplace_back(v, i * 2);
        tree[v].emplace_back(u, i * 2 + 1);
    }
    std::vector<std::array<std::uint64_t, 2>> paths(walks.size());
    for (std::size_t i = 0; i < walks.size(); ++i)
    {
        const auto [source, target] = walks[i];
        std::vector<int> stack{source};
        std::vector<bool> seen(n);
        seen[source] = true;
        std::vector<std::uint64_t> masks(n);
        while (!stack.empty())
        {
            const int u = stack.back();
            stack.pop_back();
            for (const auto &[v, bit] : tree[u])
            {
                if (!seen[v])
                {
                    seen[v] = true;
                    masks[v] = masks[u] | (std::uint64_t{1} << bit);
                    stack.push_back(v);
                }
            }
        }
        const auto mask = masks[target];
        const auto reversed = ((mask & 0x5555555555555555ULL) << 1) | ((mask & 0xaaaaaaaaaaaaaaaaULL) >> 1);
        paths[i] = {mask, reversed};
    }
    int optimum = 0;
    for (unsigned sample = 0; sample < (1U << walks.size()); ++sample)
    {
        std::uint64_t bits = 0;
        for (std::size_t i = 0; i < paths.size(); ++i)
        {
            bits |= paths[i][sample >> i & 1U];
        }
        optimum = std::max(optimum, std::popcount(bits));
    }
    assert(result.covered_directions == static_cast<std::uint64_t>(optimum));
    std::uint64_t witness = 0;
    for (std::size_t i = 0; i < paths.size(); ++i)
    {
        witness |= paths[i][result.reversed[i]];
    }
    assert(std::popcount(witness) == optimum);
}

template <class Error, class Action>
void expect_error(Action action)
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
    compare(0, {}, {});
    compare(1, {}, {});
    compare(1, {}, {{0, 0}});
    compare(4, {{0, 1}, {0, 2}, {0, 3}}, {{1, 2}, {2, 3}, {3, 1}});
    compare(6, {{0, 1}, {1, 2}, {0, 3}, {3, 4}, {3, 5}}, {{1, 3}, {2, 5}, {4, 5}, {3, 4}});
    compare(5, {{0, 1}, {1, 2}, {2, 3}, {3, 4}}, {{0, 4}, {4, 0}, {4, 2}, {3, 1}, {1, 1}});
    compare(4, {{0, 1}, {1, 2}, {2, 3}}, {{0, 1}, {3, 0}, {1, 2}});
    compare(3, {{0, 1}, {1, 2}}, {{2, 0}});
    compare(3, {{0, 1}, {1, 2}}, {{0, 2}});

    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::orient_tree_walks(-1, {}, {});
                                    });
    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::orient_tree_walks(std::numeric_limits<int>::max(), {}, {});
                                    });
    expect_error<std::invalid_argument>([]
                                        {
                                            (void)sirkit::orient_tree_walks(3, {}, {});
                                        });
    expect_error<std::invalid_argument>([]
                                        {
                                            const std::array edges{Pair{1, 2}, Pair{2, 3}, Pair{3, 1}};
                                            (void)sirkit::orient_tree_walks(4, edges, {});
                                        });
    for (const auto &[u, v] : {Pair{-1, 0}, Pair{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array edges{Pair{u, v}};
                                            (void)sirkit::orient_tree_walks(2, edges, {});
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array edges{Pair{0, 1}}, walks{Pair{v, u}};
                                            (void)sirkit::orient_tree_walks(2, edges, walks);
                                        });
    }

    std::mt19937 random(2501865);
    for (int trial = 0; trial < 2100; ++trial)
    {
        const int n = static_cast<int>(random() % 10) + 1;
        std::vector<int> vertices(n);
        for (int v = 0; v < n; ++v)
        {
            vertices[v] = v;
        }
        std::shuffle(vertices.begin(), vertices.end(), random);
        std::vector<Pair> edges, walks;
        for (int v = 1; v < n; ++v)
        {
            edges.emplace_back(vertices[v], vertices[random() % v]);
        }
        const int m = static_cast<int>(random() % 10);
        for (int i = 0; i < m; ++i)
        {
            walks.emplace_back(random() % n, random() % n);
        }
        compare(n, edges, walks);
    }

    constexpr int kSize = 100003;
    std::vector<Pair> chain;
    for (int v = 1; v < kSize; ++v)
    {
        chain.emplace_back(v - 1, v);
    }
    const std::vector<Pair> walks(kSize, Pair{0, kSize - 1});
    const auto result = sirkit::orient_tree_walks(kSize, chain, walks);
    assert(result.covered_directions == 2 * (kSize - 1));
    assert(std::find(result.reversed.begin(), result.reversed.end(), true) != result.reversed.end());
    assert(std::find(result.reversed.begin(), result.reversed.end(), false) != result.reversed.end());
}
