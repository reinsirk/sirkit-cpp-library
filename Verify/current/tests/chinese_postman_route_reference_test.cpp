// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Chinese_Postman_Route.cpp"

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <limits>
#include <optional>
#include <queue>
#include <random>
#include <stdexcept>
#include <tuple>
#include <utility>
#include <vector>

namespace
{
using Edge = sirkit::PostmanEdge;

std::optional<std::uint64_t> oracle(int n, const std::vector<Edge> &edges, int start, int finish)
{
    if (n == 0)
    {
        return 0;
    }
    if (start == -1)
    {
        start = edges.empty() ? 0 : edges.front().from;
    }
    if (finish == -1)
    {
        finish = start;
    }
    const std::size_t all = (std::size_t{1} << edges.size()) - 1;
    using State = std::tuple<std::uint64_t, std::size_t, int>;
    std::priority_queue<State, std::vector<State>, std::greater<State>> pending;
    const auto infinity = std::numeric_limits<std::uint64_t>::max();
    std::vector distance(all + 1, std::vector<std::uint64_t>(n, infinity));
    distance[0][start] = 0;
    pending.emplace(0, 0, start);
    while (!pending.empty())
    {
        const auto [current, mask, vertex] = pending.top();
        pending.pop();
        if (current != distance[mask][vertex])
        {
            continue;
        }
        if (mask == all && vertex == finish)
        {
            return current;
        }
        for (std::size_t i = 0; i < edges.size(); ++i)
        {
            const auto &[u, v, weight] = edges[i];
            if (u != vertex && v != vertex)
            {
                continue;
            }
            const int next = u == vertex ? v : u;
            const auto next_mask = mask | (std::size_t{1} << i);
            if (current + weight < distance[next_mask][next])
            {
                distance[next_mask][next] = current + weight;
                pending.emplace(current + weight, next_mask, next);
            }
        }
    }
    return std::nullopt;
}

void check(int n, const std::vector<Edge> &edges, int start, int finish, std::optional<std::uint64_t> expected)
{
    const auto route = sirkit::minimum_postman_route(n, edges, start, finish);
    assert(route.has_value() == expected.has_value());
    if (!route)
    {
        return;
    }
    assert(route->cost == *expected);
    if (n == 0)
    {
        assert(route->vertices.empty() && route->edge_ids.empty());
        return;
    }
    if (start == -1)
    {
        start = edges.empty() ? 0 : edges.front().from;
    }
    if (finish == -1)
    {
        finish = start;
    }
    assert(route->vertices.size() == route->edge_ids.size() + 1);
    assert(route->vertices.front() == start && route->vertices.back() == finish);
    std::vector<int> used(edges.size());
    std::uint64_t cost = 0;
    for (std::size_t i = 0; i < route->edge_ids.size(); ++i)
    {
        const auto id = route->edge_ids[i];
        assert(id < edges.size());
        const auto [u, v, weight] = edges[id];
        const int a = route->vertices[i];
        const int b = route->vertices[i + 1];
        assert((u == a && v == b) || (u == b && v == a));
        assert(++used[id] <= 2);
        cost += weight;
    }
    assert(cost == route->cost);
    for (const auto visits : used)
    {
        assert(visits >= 1);
    }
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
    check(0, {}, -1, -1, 0);
    check(2, {}, -1, -1, 0);
    check(2, {}, 1, 1, 0);
    check(2, {}, 0, 1, std::nullopt);
    check(4, {{1, 2, 1}}, -1, -1, 2);
    check(3, {{1, 0, 3}, {2, 2, 0}}, -1, -1, std::nullopt);
    check(3, {{1, 0, 3}}, 0, 2, std::nullopt);
    check(3, {{1, 0, 3}}, 2, -1, std::nullopt);
    check(4, {{0, 1, 10}, {0, 1, 1}, {1, 2, 3}, {2, 0, 20}, {2, 3, 0}}, 0, 3, 37);

    constexpr auto kMaximum = std::numeric_limits<std::uint64_t>::max();
    check(2, {{0, 1, kMaximum}}, 0, 1, kMaximum);
    check(2, {{0, 1, kMaximum / 2}, {0, 0, 1}}, -1, -1, kMaximum);
    const auto bridge = kMaximum / 2 + 8;
    check(6, {{0, 1, bridge}, {0, 2, 1}, {0, 3, 1}, {1, 4, 1}, {1, 5, 1}}, 0, 1, bridge + 8);
    expect_error<std::overflow_error>([]
                                      {
                                          const std::vector<Edge> too_far{{0, 1, kMaximum}};
                                          (void)sirkit::minimum_postman_route(2, too_far);
                                      });
    expect_error<std::overflow_error>([]
                                      {
                                          const std::vector<Edge> too_far{{0, 1, kMaximum}, {1, 0, 1}};
                                          (void)sirkit::minimum_postman_route(2, too_far);
                                      });

    std::vector<Edge> star;
    for (int v = 1; v <= 22; ++v)
    {
        star.push_back({0, v, 1});
    }
    check(23, star, 0, 0, 44);
    star.push_back({0, 23, 1});
    expect_error<std::length_error>([&]
                                    {
                                        (void)sirkit::minimum_postman_route(24, star);
                                    });
    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::minimum_postman_route(-1, {});
                                    });
    for (const auto &[u, v] : {std::pair{-2, 0}, std::pair{2, 0}, std::pair{0, -2}, std::pair{0, 2}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::minimum_postman_route(2, {}, u, v);
                                        });
    }
    for (const auto &edge : {Edge{-1, 0, 0}, Edge{0, 2, 0}})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::minimum_postman_route(2, std::vector<Edge>{edge});
                                        });
    }

    std::mt19937 random(315823961);
    for (int trial = 0; trial < 640; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 9);
        const int m = static_cast<int>(random() % 11);
        std::vector<Edge> edges(m);
        for (auto &edge : edges)
        {
            edge.from = static_cast<int>(random() % n);
            edge.to = static_cast<int>(random() % n);
            edge.weight = random() % 10;
        }
        const int start = static_cast<int>(random() % (n + 1)) - 1;
        const int finish = static_cast<int>(random() % (n + 1)) - 1;
        check(n, edges, start, finish, oracle(n, edges, start, finish));
    }
}
