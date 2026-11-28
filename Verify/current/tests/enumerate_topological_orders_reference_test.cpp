// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Enumerate_Topological_Orders.cpp"

#include <algorithm>
#include <cassert>
#include <numeric>
#include <random>
#include <set>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

void check(int n, const std::vector<Edge> &edges, std::size_t k)
{
    std::set<std::vector<int>> all;
    std::vector<int> permutation(n), position(n);
    std::iota(permutation.begin(), permutation.end(), 0);
    do
    {
        for (int i = 0; i < n; ++i)
        {
            position[permutation[i]] = i;
        }
        if (std::ranges::all_of(edges, [&](const Edge &edge) { return position[edge.first] < position[edge.second]; }))
        {
            all.insert(permutation);
        }
    } while (std::ranges::next_permutation(permutation).found);

    const auto result = sirkit::enumerate_topological_orders(n, edges, k);
    assert(result.size() == std::min(k, all.size()));
    for (const auto &sequence : result)
    {
        assert(all.erase(sequence) == 1);
    }
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
    check(0, {}, 0);
    check(0, {}, 1);
    check(0, {}, 7);
    check(1, {{0, 0}}, 5);
    check(4, {{0, 1}, {1, 2}, {2, 1}}, 2);
    check(4, {{0, 2}, {0, 2}, {1, 2}}, 10);
    check(5, {{0, 1}, {1, 2}, {2, 3}, {3, 4}}, 10);
    std::mt19937 random(947701);
    for (int test = 0; test < 280; ++test)
    {
        const int n = static_cast<int>(random() % 8) + 1;
        std::vector<int> ordering(n);
        std::iota(ordering.begin(), ordering.end(), 0);
        std::shuffle(ordering.begin(), ordering.end(), random);
        std::vector<Edge> edges;
        for (int u = 0; u < n; ++u)
        {
            for (int v = u + 1; v < n; ++v)
            {
                if (random() % 3 == 0)
                {
                    edges.emplace_back(ordering[u], ordering[v]);
                }
            }
        }
        if (test % 3 == 0 && !edges.empty())
        {
            const auto [u, v] = edges.front();
            edges.emplace_back(v, u);
        }
        check(n, edges, random() % 12);
    }
#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::length_error>([] { (void)sirkit::enumerate_topological_orders(-1, {}, 1); });
    for (const Edge edge : std::vector<Edge>{{-1, 0}, {2, 0}, {0, -1}, {0, 2}})
    {
        check_error<std::out_of_range>(
            [&] { (void)sirkit::enumerate_topological_orders(2, std::span(&edge, 1), 1); });
    }
#endif
}
