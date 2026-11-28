// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Dynamic_Edge_Products.cpp"

#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = std::pair<int, int>;
using Value = std::int64_t;
using Graph = sirkit::DynamicEdgeProducts<Value>;

void check_operations(int n, const std::vector<Edge> &edges, std::mt19937 &random)
{
    std::vector<Value> values(n);
    for (auto &value : values)
    {
        value = static_cast<Value>(random() % 61) - 30;
    }
    Graph actual(n, edges, values);
    std::vector<bool> active(edges.size());
    for (int trial = 0; trial < 210; ++trial)
    {
        const int v = static_cast<int>(random() % n);
        if (!edges.empty() && trial % 3 != 0)
        {
            const auto id = random() % edges.size();
            const bool enabled = random() % 2 != 0;
            assert(actual.set_active(id, enabled) == (active[id] != enabled));
            active[id] = enabled;
            assert(actual.active(id) == enabled);
        }
        else
        {
            const auto delta = static_cast<Value>(random() % 29) - 14;
            values[v] += delta;
            actual.add(v, delta);
        }

        std::vector<Value> sum(n);
        Value products = 0;
        for (std::size_t id = 0; id < edges.size(); ++id)
        {
            if (!active[id])
            {
                continue;
            }
            const auto [u, w] = edges[id];
            products += values[u] * values[w];
            sum[u] += values[w];
            sum[w] += values[u];
        }
        assert(actual.sum_edge_products() == products);
        for (int u = 0; u < n; ++u)
        {
            assert(actual.neighbor_sum(u) == sum[u] && actual.value(u) == values[u]);
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
    const Graph empty(0, {});
    assert(empty.sum_edge_products() == 0);
    Graph singleton(1, {});
    singleton.add(0, -19);
    assert(singleton.value(0) == -19 && singleton.neighbor_sum(0) == 0 && singleton.sum_edge_products() == 0);
    constexpr auto kMax = std::numeric_limits<std::uint64_t>::max();
    const std::vector<Edge> two{{0, 1}};
    const std::vector<std::uint64_t> large_weights{kMax, kMax};
    sirkit::DynamicEdgeProducts<> large(2, two, large_weights);
    assert(large.set_active(0, true) && large.sum_edge_products() == 1);
    large.add(0, 2);
    assert(large.value(0) == 1 && large.sum_edge_products() == kMax);
    assert(large.set_active(0, false) && large.sum_edge_products() == 0);

    const std::vector<std::int8_t> small_weights{100, 100};
    sirkit::DynamicEdgeProducts<std::int8_t> narrow(2, two, small_weights);
    narrow.set_active(0, true);
    assert(narrow.sum_edge_products() == static_cast<std::int8_t>(10000));
    narrow.add(1, 41);
    assert(narrow.value(1) == static_cast<std::int8_t>(141));
    assert(narrow.neighbor_sum(0) == static_cast<std::int8_t>(141));
    assert(narrow.sum_edge_products() == static_cast<std::int8_t>(14100));

    std::mt19937 random(35091841);
    std::vector<Edge> heavy{{0, 1}, {2, 0}, {1, 2}};
    for (int leaf = 3; leaf < 112; ++leaf)
    {
        for (int center = 0; center < 3; ++center)
        {
            heavy.emplace_back(leaf, center);
        }
    }
    check_operations(112, heavy, random);
    for (int test = 0; test < 480; ++test)
    {
        const int n = static_cast<int>(random() % 27) + 1;
        std::vector<Edge> edges;
        for (int u = 0; u < n; ++u)
        {
            for (int v = u + 1; v < n; ++v)
            {
                const auto bits = random() % 7;
                if (bits == 0)
                {
                    edges.emplace_back(u, v);
                }
                else if (bits == 1)
                {
                    edges.emplace_back(v, u);
                }
            }
        }
        check_operations(n, edges, random);
    }

    expect_error<std::length_error>([]
                                    {
                                        Graph bad(-1, {});
                                    });
    const std::vector<Value> invalid_weights{2, 8, 9};
    expect_error<std::invalid_argument>([&]
                                        {
                                            Graph bad(2, two, invalid_weights);
                                        });
#ifndef SKIP_RUNTIME_CHECKS
    const std::vector<Edge> loop{{0, 0}}, duplicate{{0, 1}, {1, 0}};
    expect_error<std::invalid_argument>([&]
                                        {
                                            Graph bad(2, loop);
                                        });
    expect_error<std::invalid_argument>([&]
                                        {
                                            Graph bad(2, duplicate);
                                        });
    expect_error<std::out_of_range>([&]
                                    {
                                        (void)singleton.active(0);
                                    });
    expect_error<std::out_of_range>([&]
                                    {
                                        singleton.set_active(0, true);
                                    });
    for (const int id : {-1, 2})
    {
        const std::vector<Edge> from{{id, 0}}, to{{0, id}};
        expect_error<std::out_of_range>([&]
                                        {
                                            Graph bad(2, from);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            Graph bad(2, to);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)singleton.value(id);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)singleton.neighbor_sum(id);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            singleton.add(id, 2);
                                        });
    }
#endif
}
