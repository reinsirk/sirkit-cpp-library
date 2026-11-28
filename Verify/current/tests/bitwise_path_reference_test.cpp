// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Bitwise_Path.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::BitwisePathEdge;
constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();

void check(int n, const std::vector<Edge> &edges, bool directed, int source, int target)
{
    std::vector<std::vector<std::pair<int, std::uint64_t>>> adjacent(n);
    for (const auto &[u, v, mask] : edges)
    {
        adjacent[u].emplace_back(v, mask);
        if (!directed)
        {
            adjacent[v].emplace_back(u, mask);
        }
    }

    std::optional<std::uint64_t> best_or, best_and;
    std::vector<bool> seen(n);
    seen[source] = true;
    auto enumerate = [&](auto &&self, int v, std::uint64_t aggregate_or, std::uint64_t aggregate_and) -> void
    {
        if (v == target)
        {
            if (!best_or || aggregate_or < *best_or)
            {
                best_or = aggregate_or;
            }
            if (!best_and || aggregate_and > *best_and)
            {
                best_and = aggregate_and;
            }
            return;
        }
        for (const auto &[u, mask] : adjacent[v])
        {
            if (!seen[u])
            {
                seen[u] = true;
                self(self, u, aggregate_or | mask, aggregate_and & mask);
                seen[u] = false;
            }
        }
    };
    enumerate(enumerate, source, 0, kMax);

    auto verify = [&](const std::optional<sirkit::BitwisePath> &actual,
                      const std::optional<std::uint64_t> &expected, bool compute_or)
    {
        assert(actual.has_value() == expected.has_value());
        if (!actual)
        {
            return;
        }
        assert(actual->vertices.size() == actual->edge_ids.size() + 1);
        assert(actual->vertices.front() == source && actual->vertices.back() == target);
        std::vector<bool> visited(n);
        auto score = compute_or ? std::uint64_t{0} : kMax;
        visited[source] = true;
        for (std::size_t i = 0; i < actual->edge_ids.size(); ++i)
        {
            const int u = actual->vertices[i];
            const int v = actual->vertices[i + 1];
            const auto id = actual->edge_ids[i];
            assert(0 <= v && v < n && id < edges.size() && !visited[v]);
            visited[v] = true;
            const auto &edge = edges[id];
            assert((edge.from == u && edge.to == v) || (!directed && edge.from == v && edge.to == u));
            score = compute_or ? (score | edge.label) : (score & edge.label);
        }
        assert(score == *expected && actual->value == *expected);
    };
    verify(sirkit::minimum_or_path(n, edges, source, target, directed), best_or, true);
    verify(sirkit::maximum_and_path(n, edges, source, target, directed), best_and, false);
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
    check(1, {}, true, 0, 0);
    check(2, {}, true, 0, 1);
    check(2, {{0, 1, 0}}, true, 0, 1);
    check(2, {{0, 1, kMax}}, true, 1, 0);
    check(2, {{0, 1, kMax}}, false, 1, 0);
    check(3, {{0, 1, 1}, {1, 2, 2}, {0, 2, 4}}, true, 0, 2);
    const auto high = std::uint64_t{1} << 63;
    check(3, {{0, 1, high}, {1, 2, 1}, {0, 2, high | 2}, {0, 1, 7}}, false, 0, 2);
    check(5, {{0, 1, 1}, {0, 1, 2}, {1, 2, 2}, {1, 3, 1}, {3, 4, 1}, {2, 4, 2}, {0, 2, 8}, {0, 0, 32}, {3, 2, 16}}, false, 0, 4);

    std::mt19937_64 random(4085113);
    for (int trial = 0; trial < 960; ++trial)
    {
        const int n = static_cast<int>(random() % 8) + 1;
        const auto m = random() % 17;
        std::vector<Edge> edges;
        for (unsigned i = 0; i < m; ++i)
        {
            const auto mask = trial % 3 == 0 ? random() : random() % 64;
            edges.push_back({static_cast<int>(random() % n), static_cast<int>(random() % n), mask});
        }
        check(n, edges, trial % 2 == 0, static_cast<int>(random() % n), static_cast<int>(random() % n));
    }

    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::minimum_or_path(-1, {}, 0, 0);
                                    });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int id : {-1, 2})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::minimum_or_path(2, {}, id, 0);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::maximum_and_path(2, {}, 0, id);
                                        });
        const std::vector<Edge> from{{id, 0, 1}}, to{{0, id, 1}};
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::maximum_and_path(2, from, 0, 0);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::minimum_or_path(2, to, 0, 0);
                                        });
    }
#endif
}
