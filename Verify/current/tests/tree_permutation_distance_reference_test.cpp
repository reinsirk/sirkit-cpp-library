// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Permutation_Distance.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::TreePermutationEdge;
using Optimum = sirkit::TreePermutationDistance;

void check(int n, const std::vector<Edge> &edges)
{
    std::vector<std::vector<std::pair<int, std::uint64_t>>> graph(n);
    for (const auto &[u, v, weight] : edges)
    {
        graph[u].emplace_back(v, weight);
        graph[v].emplace_back(u, weight);
    }
    std::vector<std::vector<std::uint64_t>> distances(n, std::vector<std::uint64_t>(n));
    for (int source = 0; source < n; ++source)
    {
        auto visit = [&](auto &&self, int v, int parent, std::uint64_t cost) -> void
        {
            distances[source][v] = cost;
            for (const auto &[u, weight] : graph[v])
            {
                if (u != parent)
                {
                    self(self, u, v, cost + weight);
                }
            }
        };
        visit(visit, source, -1, 0);
    }
    auto score = [&](const std::vector<int> &order)
    {
        std::uint64_t total = 0;
        for (int v = 0; v < n; ++v)
        {
            total += distances[v][order[v]];
        }
        return total;
    };

    std::vector<int> order(n);
    std::iota(order.begin(), order.end(), 0);
    std::uint64_t best = 0;
    unsigned count = 0;
    do
    {
        const auto current = score(order);
        if (current > best)
        {
            best = current;
            count = 0;
        }
        count += current == best;
    } while (std::next_permutation(order.begin(), order.end()));

    const Optimum actual(n, edges);
    auto built = actual.permutation();
    assert(built.size() == static_cast<std::size_t>(n));
    assert(std::is_permutation(built.begin(), built.end(), order.begin(), order.end()));
    assert(score(built) == best && actual.maximum() == best);
    for (const auto modulus : {1U, 2U, 3U, 6U, 97U, 1'000'000'007U, std::numeric_limits<std::uint32_t>::max()})
    {
        assert(actual.optimal_count(modulus) == count % modulus);
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
    check(2, {{0, 1, 5}});
    check(3, {{0, 1, 3}, {1, 2, 5}});
    check(4, {{0, 1, 3}, {1, 2, 7}, {2, 3, 11}});
    check(4, {{0, 1, 3}, {0, 2, 7}, {0, 3, 11}});
    check(7, {{0, 1, 1}, {0, 2, 2}, {0, 3, 3}, {1, 4, 5}, {2, 5, 7}, {3, 6, 9}});

    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const std::vector<Edge> valid{{0, 1, kMax / 2}};
    const std::vector<Edge> invalid_sum{{0, 1, kMax / 3}, {1, 2, kMax / 3}};
    const std::vector<Edge> invalid_edge{{0, 1, kMax}};
    const std::vector<Edge> early_overflow{{0, 1, 1}, {0, 2, 1}, {0, 3, kMax}};
    const Optimum not_overflow(2, valid), sum_overflow(3, invalid_sum), edge_overflow(2, invalid_edge);
    const Optimum more_edges_after_overflow(4, early_overflow);
    assert(not_overflow.maximum() == kMax - 1 && not_overflow.optimal_count() == 1);
    assert(!sum_overflow.maximum() && sum_overflow.optimal_count() == 3);
    assert(!edge_overflow.maximum() && edge_overflow.optimal_count() == 1);
    assert(!more_edges_after_overflow.maximum() && more_edges_after_overflow.optimal_count() == 11);
    assert((edge_overflow.permutation() == std::vector<int>{1, 0}));

    std::mt19937 random(872569);
    for (int trial = 0; trial < 620; ++trial)
    {
        const int n = static_cast<int>(random() % 8) + 1;
        std::vector<int> labels(n);
        std::iota(labels.begin(), labels.end(), 0);
        std::shuffle(labels.begin(), labels.end(), random);
        std::vector<Edge> edges;
        for (int v = 1; v < n; ++v)
        {
            edges.push_back({labels[v], labels[random() % v], random() % 39 + 1});
        }
        std::shuffle(edges.begin(), edges.end(), random);
        check(n, edges);
    }

    expect_error<std::length_error>([]
                                    {
                                        Optimum bad(-1, {});
                                    });
    expect_error<std::length_error>([]
                                    {
                                        Optimum bad(2, {});
                                    });
    expect_error<std::length_error>([&]
                                    {
                                        Optimum bad(0, valid);
                                    });
    expect_error<std::invalid_argument>([&]
                                        {
                                            (void)not_overflow.optimal_count(0);
                                        });
    const std::vector<Edge> zero_weight{{0, 1, 0}};
    expect_error<std::invalid_argument>([&]
                                        {
                                            Optimum bad(2, zero_weight);
                                        });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int bad : {-1, 2})
    {
        const std::vector<Edge> from{{bad, 0}}, to{{0, bad}};
        expect_error<std::out_of_range>([&]
                                        {
                                            Optimum bad_tree(2, from);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            Optimum bad_tree(2, to);
                                        });
    }
    const std::vector<Edge> cycle{{0, 1}, {1, 2}, {2, 0}}, disconnected{{1, 2}, {2, 3}, {3, 1}};
    expect_error<std::invalid_argument>([&]
                                        {
                                            Optimum bad(4, cycle);
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            Optimum bad(4, disconnected);
                                        });
#endif
}
