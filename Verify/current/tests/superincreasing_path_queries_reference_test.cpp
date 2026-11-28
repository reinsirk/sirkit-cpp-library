// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Superincreasing_Path_Queries.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::SuperincreasingPathEdge<std::int64_t>;
using Queries = sirkit::SuperincreasingPathQueries<std::int64_t>;

std::optional<std::int64_t> oracle(const std::vector<Edge> &edges, int start, int finish,
                                   std::optional<std::size_t> removed)
{
    std::optional<std::pair<std::uint64_t, std::int64_t>> best;
    const auto visit = [&](auto &&self, int vertex, unsigned seen, std::uint64_t order, std::int64_t value) -> void
    {
        if (vertex == finish)
        {
            if (!best || order < best->first)
            {
                best = std::pair{order, value};
            }
            return;
        }
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            if (removed == i)
            {
                continue;
            }
            const auto [u, v, weight] = edges[i];
            if (vertex != u && vertex != v)
            {
                continue;
            }
            const int next = vertex == u ? v : u;
            const unsigned bit = 1U << next;
            if (!(seen & bit))
            {
                self(self, next, seen | bit, order + (std::uint64_t{1} << i), value + weight);
            }
        }
    };
    visit(visit, start, 1U << start, 0, 0);
    if (best)
    {
        return best->second;
    }
    return std::nullopt;
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
    const Queries empty(0, {});
    expect_error<std::out_of_range>([&]
                                    {
                                        (void)empty.path_sum(0, 0);
                                    });
    const Queries isolated(2, {});
    assert(isolated.path_sum(1, 1) == 0 && !isolated.path_sum(0, 1));
    expect_error<std::out_of_range>([&]
                                    {
                                        (void)isolated.path_sum_without_edge(0, 1, 0);
                                    });
    expect_error<std::length_error>([]
                                    {
                                        (void)Queries(-1, {});
                                    });
    for (const auto &[from, to] : {std::pair{-1, 0}, std::pair{2, 0}, std::pair{0, -1}, std::pair{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array invalid{Edge{from, to, 0}};
                                            (void)Queries(2, invalid);
                                        });
    }

    const std::vector<Edge> edges{{1, 0, 2}, {1, 2, 4}, {0, 2, 8}, {2, 3, 16}, {0, 2, -3}, {3, 3, -9}};
    const Queries path(5, edges);
    assert(path.path_sum(0, 2) == 6);
    assert(path.path_sum_without_edge(0, 1, 0) == 12);
    assert(path.path_sum_without_edge(1, 0, 0) == 12);
    assert(path.path_sum_without_edge(2, 3, 0) == 16);
    assert(path.path_sum_without_edge(0, 3, 2) == 22);
    assert(!path.path_sum_without_edge(0, 3, 3));
    assert(!path.path_sum_without_edge(0, 4, 3));
    const auto copy = path;
    assert(copy.path_sum_without_edge(0, 1, 0) == 12);
    expect_error<std::out_of_range>([&]
                                    {
                                        (void)copy.path_sum(-1, 0);
                                    });
    expect_error<std::out_of_range>([&]
                                    {
                                        (void)copy.path_sum_without_edge(0, 5, 0);
                                    });

    std::mt19937 random(2016071);
    for (int trial = 0; trial < 530; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 8);
        const std::size_t m = random() % 13;
        std::vector<Edge> links;
        for (std::size_t i = 0; i < m; ++i)
        {
            links.push_back({static_cast<int>(random() % n), static_cast<int>(random() % n),
                             static_cast<std::int64_t>(random() % 71) - 35});
        }
        const Queries queries(n, links);
        for (int start = 0; start < n; ++start)
        {
            const int finish = static_cast<int>(random() % n);
            assert(queries.path_sum(start, finish) == oracle(links, start, finish, {}));
            for (std::size_t removed = 0; removed < m; ++removed)
            {
                assert(queries.path_sum_without_edge(start, finish, removed) == oracle(links, start, finish, removed));
            }
        }
    }
}
