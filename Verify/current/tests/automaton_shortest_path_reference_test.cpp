// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Automaton_Shortest_Path.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Edge = sirkit::AutomatonPathEdge;
using Distance = std::optional<std::uint64_t>;
constexpr auto kMaximum = std::numeric_limits<std::uint64_t>::max();

void relax(Distance &best, std::uint64_t distance)
{
    if (!best || distance < *best)
    {
        best = distance;
    }
}

void check(
    int n, const std::vector<Edge> &edges, int from, int to, int start, const std::vector<std::vector<int>> &transition,
    const std::vector<bool> &accept, bool undirected)
{
    const int states = static_cast<int>(transition.size());
    auto id = [n](int vertex, int state) { return state * n + vertex; };
    const int size = n * states;
    std::vector distance(size, std::vector<Distance>(size));
    for (int x = 0; x < size; ++x)
    {
        distance[x][x] = 0;
    }
    for (const auto &edge : edges)
    {
        for (int state = 0; state < states; ++state)
        {
            const int destination = transition[state][edge.label];
            if (destination != -1)
            {
                relax(distance[id(edge.from, state)][id(edge.to, destination)], edge.weight);
                if (undirected)
                {
                    relax(distance[id(edge.to, state)][id(edge.from, destination)], edge.weight);
                }
            }
        }
    }
    for (int k = 0; k < size; ++k)
    {
        for (int u = 0; u < size; ++u)
        {
            for (int v = 0; v < size; ++v)
            {
                if (distance[u][k] && distance[k][v] && *distance[u][k] <= kMaximum - *distance[k][v])
                {
                    relax(distance[u][v], *distance[u][k] + *distance[k][v]);
                }
            }
        }
    }
    Distance best;
    for (int state = 0; state < states; ++state)
    {
        const auto candidate = distance[id(from, start)][id(to, state)];
        if (accept[state] && candidate)
        {
            relax(best, *candidate);
        }
    }
    auto step = [&](int state, int label) { return transition[state][label]; };
    auto allowed = [&](int state) { return accept[state]; };
    const auto result = sirkit::automaton_shortest_path(n, edges, from, to, states, start, step, allowed, undirected);
    assert(best.has_value() == result.has_value());
    if (!result)
    {
        return;
    }
    assert(result->distance == best);
    assert(result->vertices.size() == result->states.size());
    assert(result->vertices.size() == result->edges.size() + 1);
    assert(result->vertices.front() == from && result->vertices.back() == to);
    assert(result->states.front() == start && accept[result->states.back()]);
    std::uint64_t length = 0;
    for (std::size_t i = 0; i < result->edges.size(); ++i)
    {
        assert(result->edges[i] < edges.size());
        const auto &edge = edges[result->edges[i]];
        const int u = result->vertices[i];
        const int v = result->vertices[i + 1];
        assert((u == edge.from && v == edge.to) || (undirected && u == edge.to && v == edge.from));
        assert(step(result->states[i], edge.label) == result->states[i + 1]);
        assert(edge.weight <= kMaximum - length);
        length += edge.weight;
    }
    assert(length == best);
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
    check(1, {}, 0, 0, 0, {{0}}, {true}, false);
    check(1, {}, 0, 0, 0, {{0}}, {false}, true);
    check(2, {{0, 0, 0, 1}, {1, 0, 1, 1}}, 0, 1, 0, {{1, -1}, {0, 1}}, {false, true}, true);
    check(4, {{0, 1, 0, 4}, {0, 2, 0, 1}, {2, 1, 0, 1}, {1, 3, 0, 10}}, 0, 3, 0, {{0}}, {true}, false);
    check(3, {{0, 1, 0, kMaximum}, {1, 2, 0, 0}}, 0, 2, 0, {{0}}, {true}, false);
    check(3, {{0, 1, 0, kMaximum}, {1, 2, 0, 1}}, 0, 2, 0, {{0}}, {true}, false);
    check(3, {{0, 1, 0, 1}, {0, 2, 0, 1}, {1, 2, 1, 1}}, 0, 2, 0, {{0, -1}}, {true}, false);

    std::mt19937 random(796596);
    for (int test = 0; test < 1200; ++test)
    {
        const int n = static_cast<int>(random() % 7) + 1;
        const int states = static_cast<int>(random() % 4) + 1;
        const int m = static_cast<int>(random() % 35);
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            edges.push_back({static_cast<int>(random() % n), static_cast<int>(random() % n),
                             static_cast<int>(random() % 4), test % 3 == 0 ? 1 : random() % 11});
        }
        std::vector step(states, std::vector<int>(4));
        std::vector<bool> accept(states);
        for (int state = 0; state < states; ++state)
        {
            for (auto &next : step[state])
            {
                next = static_cast<int>(random() % (states + 1)) - 1;
            }
            accept[state] = random() % 3 != 0;
        }
        check(n, edges, static_cast<int>(random() % n), static_cast<int>(random() % n), static_cast<int>(random() % states),
              step, accept, random() % 2 == 0);
    }

    auto keep = [](int state, int) { return state; };
    auto accept = [](int) { return true; };
    constexpr int kLarge = std::numeric_limits<int>::max();
    check_error<std::length_error>([&] { (void)sirkit::automaton_shortest_path(kLarge, {}, 0, 0, kLarge, 0, keep, accept); });
#ifndef SKIP_RUNTIME_CHECKS
    for (const auto &[u, v] : std::vector<std::pair<int, int>>{{-1, 0}, {2, 0}, {0, -1}, {0, 2}})
    {
        check_error<std::out_of_range>([&] { (void)sirkit::automaton_shortest_path(2, {}, u, v, 1, 0, keep, accept); });
        const Edge invalid{u, v, 0, 1};
        check_error<std::out_of_range>(
            [&] { (void)sirkit::automaton_shortest_path(2, std::span(&invalid, 1), 0, 1, 1, 0, keep, accept); });
    }
    for (const auto &[count, initial] : std::vector<std::pair<int, int>>{{0, 0}, {-1, 0}, {1, -1}, {1, 1}})
    {
        check_error<std::out_of_range>(
            [&] { (void)sirkit::automaton_shortest_path(2, {}, 0, 1, count, initial, keep, accept); });
    }
    for (const std::uint64_t weight : {1, 4})
    {
        const Edge edge{0, 1, 0, weight};
        for (const int invalid : {-2, 1})
        {
            auto step = [invalid](int, int) { return invalid; };
            check_error<std::out_of_range>(
                [&] { (void)sirkit::automaton_shortest_path(2, std::span(&edge, 1), 0, 1, 1, 0, step, accept); });
        }
    }
#endif
}
