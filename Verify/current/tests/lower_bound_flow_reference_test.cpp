// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Lower_Bound_Flow.cpp"

#include <algorithm>
#include <cassert>
#include <cstdint>
#include <limits>
#include <random>
#include <stdexcept>
#include <string_view>
#include <vector>

namespace
{
struct Bounds
{
    int from, to, lower, upper;
};

struct Extremum
{
    int minimum = std::numeric_limits<int>::max();
    int maximum = -1;
};

void enumerate(const std::vector<Bounds> &edges, std::size_t id, int s, int t, int limit,
               std::vector<int> &net, Extremum &result)
{
    if (id == edges.size())
    {
        const int value = net[s];
        if (value < 0 || value > limit || net[t] != -value)
        {
            return;
        }
        for (int v = 0; v < static_cast<int>(net.size()); ++v)
        {
            if (v != s && v != t && net[v] != 0)
            {
                return;
            }
        }
        result.minimum = std::min(result.minimum, value);
        result.maximum = std::max(result.maximum, value);
        return;
    }
    const auto [u, v, lower, upper] = edges[id];
    for (int flow = lower; flow <= upper; ++flow)
    {
        net[u] += flow;
        net[v] -= flow;
        enumerate(edges, id + 1, s, t, limit, net, result);
        net[u] -= flow;
        net[v] += flow;
    }
}

void validate(int n, const std::vector<Bounds> &edges, const std::vector<int> &values, int s, int t, int flow)
{
    assert(values.size() == edges.size());
    std::vector<int> balance(n);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto [u, v, lower, upper] = edges[i];
        assert(lower <= values[i] && values[i] <= upper);
        balance[u] += values[i];
        balance[v] -= values[i];
    }
    for (int v = 0; v < n; ++v)
    {
        assert(balance[v] == (v == s ? flow : v == t ? -flow
                                                     : 0));
    }
}

void check(int n, const std::vector<Bounds> &edges, int s, int t, int limit)
{
    sirkit::LowerBoundFlow<int> plan(n);
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto [u, v, lo, hi] = edges[i];
        assert(plan.add_edge(u, v, lo, hi) == i);
    }
    assert(plan.size() == n && plan.edge_count() == edges.size());
    std::vector<int> balance(n);
    Extremum circulation_answer;
    enumerate(edges, 0, s, t, 0, balance, circulation_answer);
    const auto circulation = plan.circulation();
    assert(circulation.has_value() == (circulation_answer.maximum == 0));
    if (circulation)
    {
        validate(n, edges, *circulation, s, t, 0);
    }
    Extremum expected;
    enumerate(edges, 0, s, t, limit, balance, expected);
    const auto minimum = plan.min_flow(s, t, limit);
    const auto maximum = plan.max_flow(s, t, limit);
    assert(minimum.has_value() == (expected.maximum >= 0));
    assert(maximum.has_value() == minimum.has_value());
    if (minimum)
    {
        assert(minimum->value == expected.minimum && maximum->value == expected.maximum);
        validate(n, edges, minimum->edge_flow, s, t, minimum->value);
        validate(n, edges, maximum->edge_flow, s, t, maximum->value);
    }
}

template <class Exception, class Operation>
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

void check_limits()
{
    using Graph = sirkit::LowerBoundFlow<int>;
    constexpr int kMaximum = std::numeric_limits<int>::max();
    assert(Graph(0).circulation()->empty());
    check(2, {{0, 1, 1, 2}}, 0, 1, 0);
    check(2, {{0, 1, 1, 2}}, 0, 1, 10);
    check(2, {{1, 0, 1, 2}}, 0, 1, 10);
    check(3, {{0, 1, 0, 3}, {1, 2, 1, 3}, {1, 0, 1, 3}, {0, 2, 2, 3}}, 0, 2, 5);
    Graph widest(2);
    widest.add_edge(0, 0, kMaximum, kMaximum);
    widest.add_edge(0, 1, kMaximum, kMaximum);
    assert(widest.max_flow(0, 1)->value == kMaximum);
    assert(widest.min_flow(0, 1)->value == kMaximum);

    Graph overflow(4);
    overflow.add_edge(0, 1, kMaximum, kMaximum);
    overflow.add_edge(2, 3, kMaximum, kMaximum);
    must_throw<std::overflow_error>([&]
                                    {
                                        static_cast<void>(overflow.circulation());
                                    });
    Graph one_vertex(3);
    one_vertex.add_edge(0, 1, kMaximum, kMaximum);
    one_vertex.add_edge(0, 2, 1, 1);
    must_throw<std::overflow_error>([&]
                                    {
                                        static_cast<void>(one_vertex.circulation());
                                    });
    must_throw<std::length_error>([]
                                  {
                                      static_cast<void>(Graph(-1));
                                  });
    must_throw<std::length_error>([&]
                                  {
                                      static_cast<void>(Graph(kMaximum - 1));
                                  });

    sirkit::LowerBoundFlow<std::uint64_t> unsigned_plan(2);
    unsigned_plan.add_edge(0, 1, 1, 2);
    assert(unsigned_plan.max_flow(0, 1)->value == 2);
#ifndef SKIP_RUNTIME_CHECKS
    Graph checked(2);
    for (int invalid : {-1, 2})
    {
        must_throw<std::out_of_range>([&]
                                      {
                                          checked.add_edge(invalid, 0, 0, 1);
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          checked.add_edge(0, invalid, 0, 1);
                                      });
        must_throw<std::out_of_range>([&]
                                      {
                                          static_cast<void>(checked.max_flow(0, invalid));
                                      });
    }
    must_throw<std::invalid_argument>([&]
                                      {
                                          checked.add_edge(0, 1, -1, 2);
                                      });
    must_throw<std::invalid_argument>([&]
                                      {
                                          checked.add_edge(0, 1, 2, 1);
                                      });
    must_throw<std::invalid_argument>([&]
                                      {
                                          static_cast<void>(checked.min_flow(0, 1, -1));
                                      });
    must_throw<std::invalid_argument>([&]
                                      {
                                          static_cast<void>(checked.min_flow(1, 1));
                                      });
#endif
}

void check_random()
{
    std::mt19937 rng(285);
    for (int trial = 0; trial < 1100; ++trial)
    {
        const int n = 2 + static_cast<int>(rng() % 4);
        const int m = static_cast<int>(rng() % 9);
        std::vector<Bounds> edges;
        for (int i = 0; i < m; ++i)
        {
            const int u = static_cast<int>(rng() % n);
            const int v = static_cast<int>(rng() % n);
            const int low = static_cast<int>(rng() % 3);
            const int high = low + static_cast<int>(rng() % 4);
            edges.push_back({u, v, low, high});
        }
        const int s = static_cast<int>(rng() % n);
        const int t = (s + 1 + static_cast<int>(rng() % (n - 1))) % n;
        check(n, edges, s, t, static_cast<int>(rng() % 10));
    }
}
} // namespace

int main(int argc, char **argv)
{
    if (argc == 2 && std::string_view(argv[1]) == "invalid")
    {
        static_cast<void>(sirkit::LowerBoundFlow(-1));
        return 1;
    }
    check_limits();
    check_random();
}
