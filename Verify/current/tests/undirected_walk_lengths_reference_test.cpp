// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Undirected_Walk_Lengths.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;
using Paths = sirkit::UndirectedWalkLengths;

void check_length_layers(int n, const std::vector<Edge> &edges, int source)
{
    const int limit = 3 * n + 4;
    std::vector<std::vector<std::vector<bool>>> reachable(n, std::vector<std::vector<bool>>(limit + 1, std::vector<bool>(n)));
    for (int start = 0; start < n; ++start)
    {
        reachable[start][0][start] = true;
        for (int length = 1; length <= limit; ++length)
        {
            for (const auto &[u, v] : edges)
            {
                reachable[start][length][v] = reachable[start][length][v] || reachable[start][length - 1][u];
                reachable[start][length][u] = reachable[start][length][u] || reachable[start][length - 1][v];
            }
        }
    }
    const Paths actual(n, edges, source);
    for (int target = 0; target < n; ++target)
    {
        std::array<std::int64_t, 2> first{-1, -1};
        for (int length = 0; length <= limit; ++length)
        {
            const auto found = reachable[source][length][target];
            assert(actual.can_walk_to(target, length) == found);
            if (found && first[length % 2] < 0)
            {
                first[length % 2] = length;
            }

            std::vector<int> expected;
            for (int v = 0; v < n; ++v)
            {
                for (int split = 0; split <= length; ++split)
                {
                    if (reachable[source][split][v] && reachable[v][length - split][target])
                    {
                        expected.push_back(v);
                        break;
                    }
                }
            }
            assert(actual.vertices_on_walks(target, length) == expected);
        }
        assert(actual.minimum_lengths(target) == first);
    }
}

template <typename Error, typename Action>
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
    check_length_layers(1, {}, 0);
    check_length_layers(1, {{0, 0}}, 0);
    check_length_layers(5, {{0, 1}, {1, 2}, {2, 3}, {0, 1}}, 0);
    check_length_layers(7, {{0, 1}, {0, 2}, {1, 2}, {2, 3}, {3, 4}, {3, 5}}, 1);

    const std::vector<Edge> triangle{{0, 1}, {1, 2}, {2, 0}}, path{{0, 1}, {1, 2}};
    const Paths odd(3, triangle, 0);
    const Paths even(4, path, 0);
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    assert(odd.can_walk_to(0, kMax) && odd.can_walk_to(0, kMax - 1));
    assert((odd.vertices_on_walks(1, kMax) == std::vector<int>{0, 1, 2}));
    assert(even.can_walk_to(2, kMax - 1) && !even.can_walk_to(2, kMax) && !even.can_walk_to(3, kMax - 1));
    assert((even.vertices_on_walks(2, kMax - 1) == std::vector<int>{0, 1, 2}));

    std::mt19937 random(1190673);
    for (int trial = 0; trial < 660; ++trial)
    {
        const int n = static_cast<int>(random() % 9) + 1;
        const auto m = random() % (n * n + 1);
        std::vector<Edge> edges;
        for (unsigned i = 0; i < m; ++i)
        {
            edges.emplace_back(static_cast<int>(random() % n), static_cast<int>(random() % n));
        }
        check_length_layers(n, edges, static_cast<int>(random() % n));
    }

    expect_error<std::length_error>([]
                                    {
                                        Paths bad(-1, {}, 0);
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int id : {-1, 4})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            Paths bad(4, {}, id);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)even.minimum_lengths(id);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)even.vertices_on_walks(id, 10);
                                        });
        const std::vector<Edge> wrong_from{{id, 0}}, wrong_to{{0, id}};
        expect_error<std::out_of_range>([&]
                                        {
                                            Paths bad(4, wrong_from, 0);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            Paths bad(4, wrong_to, 0);
                                        });
    }
#endif
}
