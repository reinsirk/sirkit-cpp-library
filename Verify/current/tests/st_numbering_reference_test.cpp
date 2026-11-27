// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/ST_Numbering.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <initializer_list>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;
using Graph = std::vector<std::vector<int>>;

bool check_paths(const Graph &graph, int source, int sink, const std::vector<int> &ranks)
{
    const auto n = static_cast<int>(graph.size());
    if (ranks.size() != graph.size() || ranks[source] != 0 || ranks[sink] != n - 1)
    {
        return false;
    }
    std::vector<int> order(n, -1);
    for (int v = 0; v < n; ++v)
    {
        const int rank = ranks[v];
        if (rank < 0 || rank >= n || order[rank] != -1)
        {
            return false;
        }
        order[rank] = v;
    }
    for (const bool reverse : {false, true})
    {
        std::vector<bool> reached(n);
        reached[reverse ? sink : source] = true;
        for (int i = 0; i < n; ++i)
        {
            const int v = order[reverse ? n - 1 - i : i];
            if (!reached[v])
            {
                return false;
            }
            for (int to : graph[v])
            {
                if (reverse ? ranks[v] > ranks[to] : ranks[v] < ranks[to])
                {
                    reached[to] = true;
                }
            }
        }
    }
    return true;
}

bool reference(const Graph &graph, int source, int sink)
{
    const int n = static_cast<int>(graph.size());
    std::vector<int> middle;
    for (int v = 0; v < n; ++v)
    {
        if (v != source && v != sink)
        {
            middle.push_back(v);
        }
    }
    std::vector<int> ranks(n);
    ranks[source] = 0;
    ranks[sink] = n - 1;
    do
    {
        for (int i = 0; i < n - 2; ++i)
        {
            ranks[middle[i]] = i + 1;
        }
        if (check_paths(graph, source, sink, ranks))
        {
            return true;
        }
    } while (std::next_permutation(middle.begin(), middle.end()));
    return false;
}

void check(int n, int source, int sink, const std::vector<Edge> &edges)
{
    Graph graph(n);
    for (const auto &[u, v] : edges)
    {
        graph[u].push_back(v);
        graph[v].push_back(u);
    }
    const auto answer = sirkit::st_numbering(n, source, sink, edges);
    assert(answer.has_value() == reference(graph, source, sink));
    if (answer)
    {
        assert(check_paths(graph, source, sink, *answer));
    }
}

template <class Exception>
void expect_error(int n, int source, int sink, std::initializer_list<Edge> edges)
{
    try
    {
        static_cast<void>(sirkit::st_numbering(n, source, sink, {edges.begin(), edges.size()}));
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
} // namespace

int main()
{
    using sirkit::st_numbering;
    assert(!st_numbering(0, 0, 1, {}));
    assert(!st_numbering(1, 0, 0, {}));
    assert(!st_numbering(2, 0, 0, {}));
    check(2, 0, 1, {{0, 0}, {1, 1}});
    check(2, 0, 1, {{0, 1}, {0, 1}, {1, 0}, {1, 1}});
    check(4, 0, 3, {{0, 1}, {1, 2}, {2, 3}, {0, 0}, {2, 1}});

    for (int n = 2; n <= 5; ++n)
    {
        std::vector<Edge> possible;
        for (int u = 0; u < n; ++u)
        {
            for (int v = u + 1; v < n; ++v)
            {
                possible.emplace_back(u, v);
            }
        }
        for (unsigned mask = 0; mask < (1U << possible.size()); ++mask)
        {
            std::vector<Edge> edges;
            for (std::size_t i = 0; i < possible.size(); ++i)
            {
                if ((mask & (1U << i)) != 0)
                {
                    edges.push_back(possible[i]);
                }
            }
            for (int source = 0; source < n; ++source)
            {
                for (int sink = 0; sink < n; ++sink)
                {
                    if (source != sink)
                    {
                        check(n, source, sink, edges);
                    }
                }
            }
        }
    }

    std::mt19937 random(30715);
    for (int n = 6; n <= 8; ++n)
    {
        for (int trial = 0; trial < 600; ++trial)
        {
            const int source = static_cast<int>(random() % n);
            const int sink = (source + 1 + static_cast<int>(random() % (n - 1))) % n;
            const auto m = random() % (n * n + 1);
            std::vector<Edge> edges;
            for (unsigned i = 0; i < m; ++i)
            {
                edges.emplace_back(static_cast<int>(random() % n), static_cast<int>(random() % n));
            }
            check(n, source, sink, edges);
        }
    }

    expect_error<std::length_error>(-1, 0, 1, {});
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::out_of_range>(2, -1, 1, {});
    expect_error<std::out_of_range>(2, 2, 1, {});
    expect_error<std::out_of_range>(2, 0, -1, {});
    expect_error<std::out_of_range>(2, 0, 2, {});
    expect_error<std::out_of_range>(2, 0, 1, {{-1, 1}});
    expect_error<std::out_of_range>(2, 0, 1, {{2, 1}});
    expect_error<std::out_of_range>(2, 0, 1, {{0, -1}});
    expect_error<std::out_of_range>(2, 0, 1, {{0, 2}});
#endif

    constexpr int kSize = 48000;
    std::vector<Edge> path;
    for (int v = 1; v < kSize; ++v)
    {
        path.emplace_back(v - 1, v);
    }
    const auto ranks = st_numbering(kSize, 0, kSize - 1, path);
    assert(ranks.has_value());
    for (int v = 0; v < kSize; ++v)
    {
        assert((*ranks)[v] == v);
    }
    assert(!st_numbering(kSize, 0, kSize / 2, path));
}
