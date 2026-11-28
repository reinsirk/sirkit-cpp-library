// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Dynamic_Color_Component_Sum.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Tree = sirkit::DynamicColorComponentSum;
using Edge = Tree::Edge;
using Weight = std::int64_t;

std::int64_t brute(const std::vector<std::vector<int>> &graph, const std::vector<Weight> &weights,
                   const std::vector<int> &colors, int source)
{
    std::uint64_t sum{};
    std::vector<std::pair<int, int>> pending{{source, -1}};
    while (!pending.empty())
    {
        const auto [v, parent] = pending.back();
        pending.pop_back();
        sum += static_cast<std::uint64_t>(weights[v]);
        for (int u : graph[v])
        {
            if (u != parent && colors[u] == colors[source])
            {
                pending.emplace_back(u, v);
            }
        }
    }
    return static_cast<Weight>(sum);
}

template<class Exception, class Operation>
void expect_throw(Operation operation)
{
    bool threw = false;
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        threw = true;
    }
    assert(threw);
}

void check_contracts_and_extreme_weights()
{
    const Tree empty({}, {}, {});
    assert(empty.size() == 0);
    const std::array<Weight, 1> sole_weight{7};
    const std::array one_color{1};
    Tree single(sole_weight, one_color, {});
    assert(single.size() == 1 && single.color(0) == 1 && single.component_sum(0) == 7);
    single.set_color(0, 1);
    assert(single.toggle(0) == 0);
    single.add_weight(0, -9);
    assert(single.weight(0) == -2 && single.component_sum(0) == -2);

    constexpr Weight kMax = std::numeric_limits<Weight>::max();
    constexpr Weight kMin = std::numeric_limits<Weight>::min();
    const std::array<Weight, 3> weights{kMax, kMin, 11};
    const std::array colors{0, 0, 0};
    const std::array edges{Edge{0, 1}, Edge{1, 2}};
    Tree tree(weights, colors, edges);
    assert(tree.component_sum(2) == 10);
    tree.set_color(1, 1);
    assert(tree.component_sum(0) == kMax);
    assert(tree.component_sum(2) == 11);
    tree.set_weight(1, kMax);
    tree.add_weight(1, 1);
    assert(tree.weight(1) == kMin && tree.component_sum(1) == kMin);
    auto copy = tree;
    assert(copy.toggle(1) == 0 && copy.component_sum(0) == 10 && tree.component_sum(1) == kMin);
    single = copy;
    assert(single.component_sum(2) == 10 && single.size() == 3);
    auto moved = std::move(copy);
    assert(moved.component_sum(1) == 10 && copy.size() == 0);
    moved = std::move(tree);
    assert(moved.component_sum(2) == 11 && tree.size() == 0);

    expect_throw<std::length_error>([] {
        sirkit::detail::check_color_component_capacity(static_cast<std::size_t>(std::numeric_limits<int>::max()));
    });
#ifndef SKIP_RUNTIME_CHECKS
    expect_throw<std::out_of_range>([&] { static_cast<void>(empty.component_sum(0)); });
    expect_throw<std::out_of_range>([&] { static_cast<void>(moved.toggle(-1)); });
    expect_throw<std::invalid_argument>([&] { moved.set_color(0, -1); });
    expect_throw<std::invalid_argument>([&] { moved.set_color(0, 2); });
    expect_throw<std::invalid_argument>([&] { Tree bad(weights, {}, edges); });
    expect_throw<std::invalid_argument>([&] { Tree bad(weights, colors, {}); });
    const std::array invalid_colors{0, 0, 2};
    expect_throw<std::invalid_argument>([&] { Tree bad(weights, invalid_colors, edges); });
    for (const Edge &bad : {Edge{-1, 1}, Edge{3, 1}, Edge{0, -1}, Edge{0, 3}})
    {
        const std::array bad_edges{bad, edges[1]};
        expect_throw<std::out_of_range>([&] { Tree invalid(weights, colors, bad_edges); });
    }
    const std::array repeated{Edge{0, 1}, Edge{1, 0}};
    expect_throw<std::invalid_argument>([&] { Tree invalid(weights, colors, repeated); });
#endif
}

void test_small_random_trees()
{
    std::mt19937_64 rng(4415292);
    for (int trial = 0; trial < 850; ++trial)
    {
        const int n = 1 + rng() % 80;
        std::vector<Edge> edges;
        std::vector<std::vector<int>> graph(n);
        std::vector<int> permutation(n);
        for (int v = 0; v < n; ++v)
        {
            permutation[v] = v;
        }
        std::shuffle(permutation.begin(), permutation.end(), rng);
        for (int i = 1; i < n; ++i)
        {
            const int v = permutation[i], p = permutation[rng() % i];
            graph[v].push_back(p);
            graph[p].push_back(v);
            edges.emplace_back(v, p);
        }
        std::shuffle(edges.begin(), edges.end(), rng);
        std::vector<Weight> weights(n);
        std::vector<int> colors(n);
        for (int v = 0; v < n; ++v)
        {
            weights[v] = static_cast<Weight>(rng() % 201) - 100;
            colors[v] = rng() % 2;
        }
        Tree tree(weights, colors, edges);
        for (int step = 0; step < 100; ++step)
        {
            const int vertex = rng() % n;
            const int kind = rng() % 5;
            if (kind == 0)
            {
                colors[vertex] ^= 1;
                assert(tree.toggle(vertex) == colors[vertex]);
            }
            else if (kind == 1)
            {
                colors[vertex] = rng() % 2;
                tree.set_color(vertex, colors[vertex]);
            }
            else if (kind == 2)
            {
                const Weight delta = static_cast<Weight>(rng() % 101) - 50;
                weights[vertex] += delta;
                tree.add_weight(vertex, delta);
            }
            else if (kind == 3)
            {
                weights[vertex] = static_cast<Weight>(rng() % 1001) - 500;
                tree.set_weight(vertex, weights[vertex]);
            }
            const int query = rng() % n;
            const Tree &view = tree;
            assert(view.color(vertex) == colors[vertex] && view.weight(vertex) == weights[vertex]);
            assert(view.component_sum(query) == brute(graph, weights, colors, query));
        }
    }
}

void test_maximum_shapes()
{
    constexpr int kN = 300000;
    std::vector<Weight> weights(kN, 1);
    std::vector<int> colors(kN);
    std::vector<Edge> edges;
    edges.reserve(kN - 1);
    for (int v = 1; v < kN; ++v)
    {
        colors[v] = v % 2;
        edges.emplace_back(v, v == 1 ? 0 : 1);
    }
    {
        Tree star(weights, colors, edges);
        for (int i = 0; i < 2000; ++i)
        {
            assert(star.component_sum(1) == kN / 2);
            assert(star.component_sum(0) == 1);
            star.toggle(1);
            assert(star.component_sum(0) == kN / 2 + 1);
            assert(star.component_sum(2) == kN / 2 + 1);
            star.toggle(1);
        }
    }
    for (int v = 1; v < kN; ++v)
    {
        colors[v] = v >= kN / 2;
        edges[v - 1] = {v, v - 1};
    }
    Tree chain(weights, colors, edges);
    assert(chain.component_sum(0) == kN / 2 && chain.component_sum(kN - 1) == kN / 2);
    chain.toggle(kN / 2 - 1);
    chain.add_weight(kN - 1, 17);
    assert(chain.component_sum(0) == kN / 2 - 1);
    assert(chain.component_sum(kN - 1) == kN / 2 + 18);
}
}

int main()
{
    check_contracts_and_extreme_weights();
    test_small_random_trees();
    test_maximum_shapes();
}
