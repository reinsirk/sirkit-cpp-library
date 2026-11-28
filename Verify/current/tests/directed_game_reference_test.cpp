// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Directed_Game.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <random>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;
using Outcome = sirkit::DirectedGameOutcome;

void check(int n, const std::vector<Edge> &edges)
{
    std::vector<std::vector<int>> adjacent(n);
    for (const auto &[u, v] : edges)
    {
        adjacent[u].push_back(v);
    }
    std::vector<Outcome> current(n, Outcome::draw);
    std::vector<int> length(n, -1);
    for (int limit = 0; limit <= n; ++limit)
    {
        const auto previous = current;
        for (int u = 0; u < n; ++u)
        {
            if (std::ranges::any_of(adjacent[u], [&](int v)
                                    {
                                        return previous[v] == Outcome::lose;
                                    }))
            {
                current[u] = Outcome::win;
            }
            else if (std::ranges::all_of(adjacent[u], [&](int v)
                                         {
                                             return previous[v] == Outcome::win;
                                         }))
            {
                current[u] = Outcome::lose;
            }
            if (length[u] == -1 && current[u] != Outcome::draw)
            {
                length[u] = limit;
            }
        }
    }

    const auto answer = sirkit::solve_directed_game(n, edges);
    assert(answer.size() == static_cast<std::size_t>(n));
    for (int u = 0; u < n; ++u)
    {
        const auto [outcome, plies, next] = answer[u];
        assert(outcome == current[u] && plies == length[u]);
        if (adjacent[u].empty())
        {
            assert(next == -1);
        }
        else
        {
            assert(std::ranges::find(adjacent[u], next) != adjacent[u].end());
            if (outcome == Outcome::draw)
            {
                assert(current[next] == Outcome::draw);
            }
            else
            {
                assert(current[next] != outcome && current[next] != Outcome::draw);
                assert(length[next] == plies - 1);
            }
        }
    }
}
} // namespace

int main()
{
    check(0, {});
    check(4, {});
    check(7, {{0, 0}, {0, 1}, {2, 1}, {1, 4}, {1, 3}, {3, 4}, {5, 6}, {6, 5}, {5, 6}});
    check(5, {{0, 1}, {1, 2}, {2, 3}, {3, 4}, {0, 2}});
    std::mt19937 rng(592748);
    for (int trial = 0; trial < 1400; ++trial)
    {
        const int n = 1 + rng() % 10;
        const int m = rng() % 23;
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            edges.emplace_back(rng() % n, rng() % n);
        }
        check(n, edges);
    }

#ifndef SKIP_RUNTIME_CHECKS
    int rejected = 0;
    try
    {
        (void)sirkit::solve_directed_game(-1, {});
    }
    catch (const std::invalid_argument &)
    {
        ++rejected;
    }
    for (const auto &edge : std::array{Edge{-1, 0}, Edge{1, 0}, Edge{0, -1}, Edge{0, 1}})
    {
        try
        {
            (void)sirkit::solve_directed_game(1, std::span(&edge, 1));
        }
        catch (const std::out_of_range &)
        {
            ++rejected;
        }
    }
    assert(rejected == 5);
#endif
}
