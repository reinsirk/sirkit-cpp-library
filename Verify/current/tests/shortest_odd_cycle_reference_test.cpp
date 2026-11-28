// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Shortest_Odd_Cycle.cpp"

#include <cassert>
#include <cstddef>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

void check(int n, const std::vector<Edge> &edges)
{
    std::vector<std::vector<bool>> adjacent(n, std::vector<bool>(n));
    for (const auto &[u, v] : edges)
    {
        adjacent[u][v] = adjacent[v][u] = true;
    }
    int shortest = n + 1;
    for (int first = 0; first < n; ++first)
    {
        std::vector<bool> seen(n);
        seen[first] = true;
        auto search = [&](auto &&self, int u, int length) -> void
        {
            if (length >= shortest)
            {
                return;
            }
            if (length % 2 == 1 && adjacent[u][first])
            {
                shortest = length;
                return;
            }
            for (int v = first + 1; v < n; ++v)
            {
                if (adjacent[u][v] && !seen[v])
                {
                    seen[v] = true;
                    self(self, v, length + 1);
                    seen[v] = false;
                }
            }
        };
        search(search, first, 1);
    }
    const auto result = sirkit::shortest_odd_cycle(n, edges);
    assert(result.empty() == (shortest > n));
    if (!result.empty())
    {
        assert(result.size() == static_cast<std::size_t>(shortest));
        std::set<int> visited;
        for (std::size_t i = 0; i < result.size(); ++i)
        {
            const int u = result[i];
            const int v = result[(i + 1) % result.size()];
            assert(0 <= u && u < n && 0 <= v && v < n);
            assert(visited.insert(u).second && adjacent[u][v]);
        }
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
    check(0, {});
    check(1, {});
    check(1, {{0, 0}});
    check(2, {{0, 1}, {0, 1}, {1, 0}});
    check(4, {{0, 1}, {1, 2}, {2, 3}, {3, 0}});
    check(6, {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 1}});
    check(8, {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {4, 5}, {5, 1}, {5, 6}, {6, 7}, {7, 5}});

    for (int n = 1; n <= 5; ++n)
    {
        std::vector<Edge> possible;
        for (int u = 0; u < n; ++u)
        {
            for (int v = u + 1; v < n; ++v)
            {
                possible.emplace_back(u, v);
            }
        }
        const auto subsets = 1U << possible.size();
        for (unsigned mask = 0; mask < subsets; ++mask)
        {
            std::vector<Edge> edges;
            for (std::size_t i = 0; i < possible.size(); ++i)
            {
                if ((mask >> i) & 1U)
                {
                    edges.push_back(possible[i]);
                }
            }
            check(n, edges);
        }
    }

    std::mt19937 random(329171);
    for (int trial = 0; trial < 660; ++trial)
    {
        const int n = static_cast<int>(random() % 10) + 1;
        const auto m = random() % (n * n + 1);
        std::vector<Edge> edges;
        for (unsigned i = 0; i < m; ++i)
        {
            const int u = static_cast<int>(random() % n), v = static_cast<int>(random() % n);
            if (u != v || trial % 3 == 0)
            {
                edges.emplace_back(u, v);
            }
        }
        check(n, edges);
    }

    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::shortest_odd_cycle(-1, {});
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &bad : {Edge{-1, 0}, Edge{2, 0}, Edge{0, -1}, Edge{0, 2}})
    {
        const std::vector<Edge> edges{{0, 0}, bad};
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::shortest_odd_cycle(2, edges);
                                        });
    }
#endif
}
