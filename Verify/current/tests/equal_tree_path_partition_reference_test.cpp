// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Equal_Tree_Path_Partition.cpp"

#include <algorithm>
#include <cassert>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;

bool brute(int n, const std::vector<Edge> &edges, int length)
{
    if (n % length != 0)
    {
        return false;
    }
    // Choose exactly the edges retained in the path forest.
    for (std::size_t mask = 0; mask < (std::size_t{1} << edges.size()); ++mask)
    {
        std::vector<std::vector<int>> paths(n);
        bool has_branch = false;
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            if (((mask >> i) & 1U) == 0)
            {
                continue;
            }
            const auto [u, v] = edges[i];
            paths[u].push_back(v);
            paths[v].push_back(u);
            has_branch = has_branch || paths[u].size() > 2 || paths[v].size() > 2;
        }
        if (has_branch)
        {
            continue;
        }
        std::vector<bool> visited(n);
        auto size = [&](auto &self, int u) -> int
        {
            visited[u] = true;
            int count = 1;
            for (int v : paths[u])
            {
                if (!visited[v])
                {
                    count += self(self, v);
                }
            }
            return count;
        };
        bool valid = true;
        for (int u = 0; u < n; ++u)
        {
            if (!visited[u] && size(size, u) != length)
            {
                valid = false;
                break;
            }
        }
        if (valid)
        {
            return true;
        }
    }
    return false;
}

void check(int n, const std::vector<Edge> &edges, int length)
{
    const auto answer = sirkit::partition_forest_into_equal_paths(n, edges, length);
    assert(answer.has_value() == brute(n, edges, length));
    if (!answer)
    {
        return;
    }
    assert(answer->size() == static_cast<std::size_t>(n / length));
    std::vector<int> used(n);
    for (const auto &path : *answer)
    {
        assert(path.size() == static_cast<std::size_t>(length));
        for (std::size_t i = 0; i < path.size(); ++i)
        {
            const int vertex = path[i];
            assert(0 <= vertex && vertex < n);
            ++used[vertex];
            if (i > 0)
            {
                const int last = path[i - 1];
                assert(std::ranges::find(edges, Edge{vertex, last}) != edges.end() ||
                       std::ranges::find(edges, Edge{last, vertex}) != edges.end());
            }
        }
    }
    assert(std::ranges::all_of(used, [](int count) { return count == 1; }));
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
    check(0, {}, 3);
    check(1, {}, 1);
    check(1, {}, 2);
    check(4, {{0, 1}, {0, 2}, {2, 3}}, 4);
    check(4, {{0, 1}, {1, 2}, {1, 3}}, 4);
    check(4, {{0, 1}, {0, 2}, {0, 3}}, 4);
    check(5, {{0, 1}, {1, 2}, {0, 3}, {3, 4}}, 3);
    check(4, {{0, 1}, {2, 3}}, 2);
    std::mt19937 random(960517);
    for (int test = 0; test < 700; ++test)
    {
        const int n = static_cast<int>(random() % 11);
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            if (random() % 4 != 0 || test % 2 == 0)
            {
                const int u = static_cast<int>(random() % v);
                edges.emplace_back(u, v);
            }
        }
        for (int length = 1; length <= std::min(n + 1, 6); ++length)
        {
            check(n, edges, length);
        }
    }
#ifndef SKIP_RUNTIME_CHECKS
    check_error<std::length_error>([] { (void)sirkit::partition_forest_into_equal_paths(-1, {}, 1); });
    check_error<std::invalid_argument>([] { (void)sirkit::partition_forest_into_equal_paths(1, {}, 0); });
    for (const Edge edge : std::vector<Edge>{{-1, 0}, {2, 0}, {0, -1}, {0, 2}})
    {
        check_error<std::out_of_range>(
            [&] { (void)sirkit::partition_forest_into_equal_paths(2, std::span(&edge, 1), 1); });
    }
    for (const auto &not_forest : std::vector<std::vector<Edge>>{{{0, 0}}, {{0, 1}, {1, 0}}, {{0, 1}, {1, 2}, {2, 0}}})
    {
        check_error<std::invalid_argument>(
            [&] { (void)sirkit::partition_forest_into_equal_paths(3, not_forest, 1); });
    }
#endif
}
