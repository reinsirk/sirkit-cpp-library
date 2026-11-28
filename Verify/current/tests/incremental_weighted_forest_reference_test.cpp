// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Incremental_Weighted_Forest.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Forest = sirkit::IncrementalWeightedForest;
using Weight = std::uint64_t;
using Graph = std::vector<std::vector<std::pair<int, Weight>>>;

template<class Exception, class Operation>
void must_throw(Operation operation)
{
    bool caught = false;
    try
    {
        operation();
    }
    catch (const Exception &)
    {
        caught = true;
    }
    assert(caught);
}

std::vector<std::optional<Weight>> distances(const Graph &graph, int source)
{
    std::vector<std::optional<Weight>> answer(graph.size());
    std::queue<int> pending;
    pending.push(source);
    answer[source] = 0;
    while (!pending.empty())
    {
        const int v = pending.front();
        pending.pop();
        for (const auto &[u, w] : graph[v])
        {
            if (!answer[u])
            {
                answer[u] = *answer[v] + w;
                pending.push(u);
            }
        }
    }
    return answer;
}

void check_all(const Forest &forest, const Graph &graph)
{
    const auto n = static_cast<int>(graph.size());
    std::vector<std::vector<std::optional<Weight>>> all(n);
    for (int u = 0; u < n; ++u)
    {
        all[u] = distances(graph, u);
    }
    for (int u = 0; u < n; ++u)
    {
        int count = 0;
        Weight best = 0;
        for (int v = 0; v < n; ++v)
        {
            assert(forest.distance(u, v) == all[u][v]);
            assert(forest.connected(u, v) == all[u][v].has_value());
            if (all[u][v])
            {
                ++count;
                for (const auto &value : all[v])
                {
                    best = std::max(best, value.value_or(0));
                }
            }
        }
        const auto longest = forest.diameter(u);
        assert(forest.component_size(u) == count);
        assert(longest.distance == best);
        assert(all[u][longest.first] && all[u][longest.second]);
        assert(all[longest.first][longest.second] == best);
    }
}

void check_limits()
{
    constexpr auto kMax = std::numeric_limits<Weight>::max();
    assert(Forest(0).size() == 0);
    Forest forest(8);
    assert(forest.diameter(1).first == 1 && forest.diameter(1).second == 1);
    assert(forest.diameter(1).distance == 0 && forest.distance(1, 1) == 0);
    assert(!forest.link(1, 1, kMax) && !forest.connected(1, 2));
    assert(forest.link(0, 1, kMax - 3));
    assert(forest.link(2, 3, 4));
    must_throw<std::overflow_error>([&] { static_cast<void>(forest.link(0, 2, 0)); });
    must_throw<std::overflow_error>([&] { static_cast<void>(forest.link(0, 4, 4)); });
    assert(!forest.connected(0, 2) && forest.distance(0, 1) == kMax - 3);
    assert(forest.link(0, 4, 3));
    assert(forest.diameter(4).distance == kMax && forest.distance(4, 1) == kMax);
    assert(forest.link(0, 6, 0) && forest.distance(4, 6) == 3);

    Forest larger(8);
    for (int v = 1; v < 5; ++v)
    {
        assert(larger.link(v - 1, v, 0));
    }
    assert(larger.link(5, 6, 10));
    assert(larger.link(6, 7, 10));
    assert(larger.link(6, 2, 0));
    assert(larger.diameter(4).distance == 20);
    Forest copied(larger);
    assert(copied.component_size(0) == 8 && copied.distance(5, 7) == 20);
    forest = copied;
    assert(forest.distance(5, 7) == 20);
    forest = std::move(larger);
    assert(forest.distance(3, 7) == 10);

    must_throw<std::length_error>([] { const Forest invalid(-1); });
#ifndef SKIP_RUNTIME_CHECKS
    for (const int v : {-1, 8})
    {
        must_throw<std::out_of_range>([&] { static_cast<void>(forest.component_size(v)); });
        must_throw<std::out_of_range>([&] { static_cast<void>(forest.link(0, v, 1)); });
    }
#endif
}

void check_random()
{
    std::mt19937 rng(2296);
    for (int test = 0; test < 100; ++test)
    {
        const int n = 1 + static_cast<int>(rng() % 20);
        Forest forest(n);
        Graph graph(n);
        for (int i = 0; i < 2 * n; ++i)
        {
            const int u = static_cast<int>(rng() % n), v = static_cast<int>(rng() % n);
            const auto weight = static_cast<Weight>(rng() % 801);
            const bool succeeds = !distances(graph, u)[v];
            assert(forest.link(u, v, weight) == succeeds);
            if (succeeds)
            {
                graph[u].emplace_back(v, weight);
                graph[v].emplace_back(u, weight);
            }
            check_all(forest, graph);
        }
    }
}

void check_balanced_merge()
{
    constexpr int kVertices = 1024;
    constexpr Weight kWeight = 1'000'000'000;
    Forest forest(kVertices);
    for (int size = 1; size < kVertices; size *= 2)
    {
        for (int start = 0; start < kVertices; start += 2 * size)
        {
            assert(forest.link(start + size, start + size - 1, kWeight));
            assert(forest.diameter(start).distance == (2 * size - 1) * kWeight);
        }
    }
    assert(forest.distance(42, 913) == (913 - 42) * kWeight);
    assert(forest.distance(kVertices - 1, 0) == (kVertices - 1) * kWeight);
}
}

int main()
{
    check_limits();
    check_random();
    check_balanced_merge();
}
