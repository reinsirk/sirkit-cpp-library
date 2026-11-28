// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Tree_Distance_Sums.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <numeric>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using T = std::int64_t;
using Edge = sirkit::TreeDistanceSumEdge<T>;
using Tree = sirkit::TreeDistanceSums<T>;

void check_random(int n, const std::vector<Edge> &edges, std::vector<T> weights, int root,
                  std::mt19937 &random)
{
    std::vector<std::vector<std::pair<int, T>>> adjacency(n);
    for (const auto &[u, v, length] : edges)
    {
        adjacency[u].emplace_back(v, length);
        adjacency[v].emplace_back(u, length);
    }
    std::vector<std::vector<T>> distance(n, std::vector<T>(n));
    std::vector<std::vector<int>> parent(n, std::vector<int>(n));
    for (int source = 0; source < n; ++source)
    {
        auto visit = [&](auto &&self, int v, int previous, T length) -> void
        {
            distance[source][v] = length;
            parent[source][v] = previous;
            for (const auto &[u, cost] : adjacency[v])
            {
                if (u != previous)
                {
                    self(self, u, v, length + cost);
                }
            }
        };
        visit(visit, source, -1, 0);
    }
    auto expected_at = [&](int v)
    {
        return std::inner_product(weights.begin(), weights.end(), distance[v].begin(), T{0});
    };

    Tree actual(n, edges, weights, root);
    for (int query = 0; query < 75; ++query)
    {
        const int u = static_cast<int>(random() % n);
        const int v = static_cast<int>(random() % n);
        if (random() % 3 == 0)
        {
            const auto delta = static_cast<T>(random() % 31) - 15;
            weights[u] += delta;
            actual.add(u, delta);
        }
        assert(actual.sum_at(u) == expected_at(u));
        assert(actual.sum_at(v) == expected_at(v));
        T total = expected_at(v);
        for (int now = u; now != v; now = parent[v][now])
        {
            total += expected_at(now);
        }
        assert(actual.sum_path(u, v) == total && actual.sum_path(v, u) == total);
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
    const Tree empty(0, {});
    Tree unit(1, {});
    unit.add(0, -19);
    assert(unit.sum_at(0) == 0 && unit.sum_path(0, 0) == 0);

    const std::vector<Edge> line{{0, 1, 2}, {1, 2, 4}, {2, 3, -1}};
    const std::vector<T> init{1, 2, 3, 4};
    Tree initial(4, line, init, 1);
    assert(initial.sum_at(1) == 26);
    assert(initial.sum_at(3) == 8);
    assert(initial.sum_path(1, 3) == 26 + 10 + 8);
    initial.add(1, 4);
    initial.add(3, -9);
    assert(initial.sum_at(1) == -1);
    assert(initial.sum_at(3) == 20);
    assert(initial.sum_path(1, 3) == -1 + 35 + 20);

    std::vector<sirkit::TreeDistanceSumEdge<std::int8_t>> short_edges{{0, 1, 120}, {1, 2, 120}};
    const std::vector<std::int8_t> short_weights{100, 121, -127};
    sirkit::TreeDistanceSums<std::int8_t> narrow(3, short_edges, short_weights);
    assert(narrow.sum_at(0) == static_cast<std::int8_t>(121 * 120 - 127 * 240));
    assert(narrow.sum_path(0, 2) == static_cast<std::int8_t>((100 - 127) * 360 + 121 * 240));
    narrow.add(0, 31);
    assert(narrow.sum_at(2) == static_cast<std::int8_t>(131 * 240 + 121 * 120));
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const std::vector<sirkit::TreeDistanceSumEdge<std::uint64_t>> large_edge{{0, 1, kMax}};
    sirkit::TreeDistanceSums<> large(2, large_edge);
    large.add(0, kMax);
    assert(large.sum_at(1) == 1);
    large.add(1, 2);
    assert(large.sum_path(0, 1) == kMax);

    std::mt19937 random(20261128);
    for (int trial = 0; trial < 590; ++trial)
    {
        const int n = static_cast<int>(random() % 25) + 1;
        std::vector<Edge> edges;
        std::vector<T> weights(n);
        for (int v = 1; v < n; ++v)
        {
            const int p = static_cast<int>(random() % v);
            const T length = static_cast<T>(random() % 29) - 14;
            if (trial % 2 == 0)
            {
                edges.push_back({v, p, length});
            }
            else
            {
                edges.push_back({p, v, length});
            }
        }
        for (auto &weight : weights)
        {
            weight = static_cast<T>(random() % 23) - 11;
        }
        check_random(n, edges, weights, static_cast<int>(random() % n), random);
    }

    expect_error<std::length_error>([]
                                    {
                                        Tree bad(-1, {});
                                    });
    expect_error<std::invalid_argument>([]
                                        {
                                            Tree bad(2, {});
                                        });
    const std::vector<T> wrong_size{1};
    expect_error<std::invalid_argument>([&]
                                        {
                                            Tree bad(4, line, wrong_size);
                                        });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int id : {-1, 4})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            Tree bad(4, line, {}, id);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            initial.add(id, 2);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)initial.sum_path(0, id);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)initial.sum_at(id);
                                        });
    }
    for (const int id : {-1, 2})
    {
        const std::vector<Edge> from{{id, 0, 1}}, to{{0, id, 1}};
        expect_error<std::out_of_range>([&]
                                        {
                                            Tree bad(2, from);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            Tree bad(2, to);
                                        });
    }
#endif
}
