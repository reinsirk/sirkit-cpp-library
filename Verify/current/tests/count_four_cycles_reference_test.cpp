// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Count_Four_Cycles.cpp"
#include <array>
#include <cassert>
#include <random>

namespace
{
using Edge = std::pair<int, int>;

void check(int n, const std::vector<Edge> &edges)
{
    std::vector<std::vector<std::vector<std::size_t>>> between(n, std::vector<std::vector<std::size_t>>(n));
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto [u, v] = edges[i];
        between[u][v].push_back(i);
        between[v][u].push_back(i);
    }
    std::vector<std::uint64_t> expected(edges.size());
    for (int a = 0; a < n; ++a)
    {
        for (int b = a + 1; b < n; ++b)
        {
            for (int c = a + 1; c < n; ++c)
            {
                for (int d = b + 1; d < n; ++d)
                {
                    if (c == b || c == d)
                    {
                        continue;
                    }
                    for (const auto w : between[a][b])
                    {
                        for (const auto x : between[b][c])
                        {
                            for (const auto y : between[c][d])
                            {
                                for (const auto z : between[d][a])
                                {
                                    for (const auto e : {w, x, y, z})
                                    {
                                        ++expected[e];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    assert(sirkit::count_four_cycles_per_edge(n, edges) == expected);
}

template <class Exception, class Operation>
void expect_error(Operation run)
{
    try
    {
        run();
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
}

int main()
{
    check(0, {});
    check(6, {{0, 0}, {0, 1}, {1, 2}, {2, 3}, {3, 0}, {3, 0}});
    std::mt19937 random(7091);
    for (int n = 1; n <= 10; ++n)
    {
        for (unsigned trial = 0; trial < 150; ++trial)
        {
            const int m = random() % 55;
            std::vector<Edge> edges(m);
            for (auto &[u, v] : edges)
            {
                u = random() % n;
                v = random() % n;
            }
            check(n, edges);
        }
    }
    const std::array<Edge, 4> cycle{{{0, 1}, {1, 2}, {2, 3}, {3, 0}}};
    constexpr std::uint64_t kCopies = 75'000;
    std::vector<Edge> many_parallel;
    for (const auto &edge : cycle)
    {
        many_parallel.insert(many_parallel.end(), kCopies, edge);
    }
    const auto result = sirkit::count_four_cycles_per_edge(4, many_parallel);
    assert(result.size() == many_parallel.size());
    assert(std::all_of(result.begin(), result.end(), [](auto x)
                       {
                           return x == kCopies * kCopies * kCopies;
                       }));
    expect_error<std::length_error>([]
                                    {
                                        sirkit::detail::CheckFourCycleEdgeCount(4'000'001);
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::invalid_argument>([]
                                        {
                                            sirkit::count_four_cycles_per_edge(-1, {});
                                        });
    for (const Edge &invalid : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            sirkit::count_four_cycles_per_edge(2, std::array{invalid});
                                        });
    }
#endif
}
