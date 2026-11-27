// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/K_Shortest_Walks.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <initializer_list>
#include <limits>
#include <queue>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::KShortestWalkEdge;
using Lengths = std::vector<std::int64_t>;

Lengths reference(int n, int source, int target, const std::vector<Edge> &edges, std::size_t k)
{
    std::vector<std::vector<std::pair<int, std::int64_t>>> graph(n);
    for (const auto &[from, to, weight] : edges)
    {
        graph[from].emplace_back(to, weight);
    }
    using State = std::pair<std::int64_t, int>;
    std::priority_queue<State, std::vector<State>, std::greater<>> queue;
    queue.emplace(0, source);
    std::vector<std::size_t> expanded(n);
    Lengths result;
    while (result.size() < k && !queue.empty())
    {
        const auto [length, vertex] = queue.top();
        queue.pop();
        if (expanded[vertex] == k)
        {
            continue;
        }
        ++expanded[vertex];
        if (vertex == target)
        {
            result.push_back(length);
        }
        for (const auto &[to, weight] : graph[vertex])
        {
            queue.emplace(length + weight, to);
        }
    }
    return result;
}

template <class Exception>
void expect_error(int n, int source, int target, std::initializer_list<Edge> edges, std::size_t k)
{
    try
    {
        static_cast<void>(sirkit::k_shortest_walk_lengths(n, source, target, {edges.begin(), edges.size()}, k));
        assert(false);
    }
    catch (const Exception &)
    {
    }
}
} // namespace

int main()
{
    using sirkit::k_shortest_walk_lengths;
    assert((k_shortest_walk_lengths(1, 0, 0, {}, 10) == Lengths{0}));
    assert(k_shortest_walk_lengths(2, 0, 1, {}, 10).empty());
    assert(k_shortest_walk_lengths(1, 0, 0, {}, 0).empty());

    const std::vector<Edge> parallel{{0, 1, 5}, {0, 1, 3}, {0, 1, 3}};
    assert((k_shortest_walk_lengths(2, 0, 1, parallel, 10) == Lengths{3, 3, 5}));
    assert((k_shortest_walk_lengths(2, 0, 1, parallel, 1) == Lengths{3}));

    const std::vector<Edge> after_target{{0, 1, 1}, {1, 2, 0}, {2, 1, 0}, {1, 1, 0}};
    assert(k_shortest_walk_lengths(3, 0, 1, after_target, 100) == Lengths(100, 1));
    assert(k_shortest_walk_lengths(3, 1, 1, after_target, 100) == Lengths(100, 0));

    const std::vector<Edge> detached_cycle{{0, 1, 1}, {0, 2, 0}, {2, 2, 0}};
    assert((k_shortest_walk_lengths(3, 0, 1, detached_cycle, 10) == Lengths{1}));

    std::mt19937 random(302891);
    for (int n = 1; n <= 9; ++n)
    {
        for (int trial = 0; trial < 450; ++trial)
        {
            const auto count = random() % (n * n + 1);
            const auto max_weight = random() % 12 + 1;
            const auto source = static_cast<int>(random() % n);
            const auto target = static_cast<int>(random() % n);
            const auto k = random() % 33;
            std::vector<Edge> edges;
            for (std::size_t i = 0; i < count; ++i)
            {
                edges.push_back({static_cast<int>(random() % n), static_cast<int>(random() % n),
                                 static_cast<std::int64_t>(random() % max_weight)});
            }
            assert(k_shortest_walk_lengths(n, source, target, edges, k) == reference(n, source, target, edges, k));
        }
    }

    constexpr auto kMax = std::numeric_limits<std::int64_t>::max();
    const std::vector<Edge> boundary{{0, 1, 0}, {0, 1, kMax}};
    assert((k_shortest_walk_lengths(2, 0, 1, boundary, 3) == Lengths{0, kMax}));

    const std::vector<Edge> wide_branch{{0, 1, 10}, {0, 2, 0}, {2, 3, kMax}, {3, 1, 1}};
    assert((k_shortest_walk_lengths(4, 0, 1, wide_branch, 1) == Lengths{10}));
    expect_error<std::overflow_error>(4, 0, 1, {{0, 1, 10}, {0, 2, 0}, {2, 3, kMax}, {3, 1, 1}}, 2);
    expect_error<std::overflow_error>(3, 0, 2, {{0, 1, kMax}, {1, 2, 1}}, 1);
    expect_error<std::overflow_error>(1, 0, 0, {{0, 0, kMax}}, 3);
    expect_error<std::invalid_argument>(0, 0, 0, {}, 1);
#ifndef SKIP_RUNTIME_CHECKS
    expect_error<std::out_of_range>(2, -1, 0, {}, 1);
    expect_error<std::out_of_range>(2, 2, 0, {}, 1);
    expect_error<std::out_of_range>(2, 0, -1, {}, 1);
    expect_error<std::out_of_range>(2, 0, 2, {}, 1);
    expect_error<std::out_of_range>(2, 0, 1, {{-1, 1, 0}}, 1);
    expect_error<std::out_of_range>(2, 0, 1, {{2, 1, 0}}, 1);
    expect_error<std::out_of_range>(2, 0, 1, {{0, -1, 0}}, 1);
    expect_error<std::out_of_range>(2, 0, 1, {{0, 2, 0}}, 1);
    expect_error<std::invalid_argument>(2, 0, 1, {{0, 1, -1}}, 1);
#endif

    // 最短路木が長いサイクルでも、木外辺を選んだ回数だけが変わる。
    constexpr int kVertices = 40000;
    constexpr std::size_t kCount = 10000;
    std::vector<Edge> cycle;
    for (int v = 0; v < kVertices; ++v)
    {
        cycle.push_back({v, (v + 1) % kVertices, 5});
    }
    const auto lengths = k_shortest_walk_lengths(kVertices, 0, kVertices - 1, cycle, kCount);
    assert(lengths.size() == kCount);
    for (std::size_t i = 0; i < kCount; ++i)
    {
        assert(lengths[i] == (static_cast<std::int64_t>(i + 1) * kVertices - 1) * 5);
    }
}
