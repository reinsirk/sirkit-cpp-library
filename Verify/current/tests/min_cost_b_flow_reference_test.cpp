// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Min_Cost_B_Flow.cpp"

#include <array>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <span>
#include <stdexcept>
#include <vector>

namespace
{
using Value = std::int64_t;
using Edge = sirkit::BFlowEdge;

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

std::optional<Value> brute_cost(const std::vector<Value> &supply, std::span<const Edge> edges)
{
    std::vector<Value> balance(supply.size());
    std::optional<Value> best;
    const auto enumerate = [&](const auto &self, std::size_t i, Value cost) -> void {
        if (i == edges.size())
        {
            if (balance == supply && (!best || cost < *best))
            {
                best = cost;
            }
            return;
        }
        const auto &edge = edges[i];
        for (Value flow = edge.lower; flow <= edge.upper; ++flow)
        {
            balance[edge.from] += flow;
            balance[edge.to] -= flow;
            self(self, i + 1, cost + flow * edge.cost);
            balance[edge.from] -= flow;
            balance[edge.to] += flow;
        }
    };
    enumerate(enumerate, 0, 0);
    return best;
}

void compare(const std::vector<Value> &supply, std::span<const Edge> edges, std::optional<Value> expected)
{
    const auto actual = sirkit::min_cost_b_flow(supply, edges);
    assert(bool(actual) == bool(expected));
    if (!actual)
    {
        return;
    }
    assert(actual->flow.size() == edges.size() && actual->potential.size() == supply.size());
    std::vector<Value> balance(supply.size());
    Value cost = 0;
    for (std::size_t i = 0; i < edges.size(); ++i)
    {
        const auto &edge = edges[i];
        const auto flow = actual->flow[i];
        assert(edge.lower <= flow && flow <= edge.upper);
        const auto reduced = edge.cost + actual->potential[edge.from] - actual->potential[edge.to];
        assert(flow == edge.lower || reduced <= 0);
        assert(flow == edge.upper || reduced >= 0);
        balance[edge.from] += flow;
        balance[edge.to] -= flow;
        cost += flow * edge.cost;
    }
    assert(balance == supply && cost == *expected);
}

void check_exceptions()
{
    constexpr auto kMin = std::numeric_limits<Value>::min();
    constexpr auto kMax = std::numeric_limits<Value>::max();
    const auto int_limit = static_cast<std::size_t>(std::numeric_limits<int>::max());
    must_throw<std::length_error>([&] { sirkit::detail::check_b_flow_capacity(int_limit, 0); });
    must_throw<std::length_error>([&] { sirkit::detail::check_b_flow_capacity(1, int_limit); });

#ifndef SKIP_RUNTIME_CHECKS
    const std::array<Value, 2> zero{};
    for (const auto &edge : {Edge{-1, 1, 0, 1, 0}, Edge{2, 1, 0, 1, 0}, Edge{0, -1, 0, 1, 0},
                            Edge{0, 2, 0, 1, 0}, Edge{0, 1, 1, 0, 0}})
    {
        must_throw<std::invalid_argument>([&] { sirkit::min_cost_b_flow(zero, std::span(&edge, 1)); });
    }
#endif

    constexpr std::array<Value, 2> empty_balance{};
    const std::array wide{Edge{0, 1, kMin, kMax, 1}};
    must_throw<std::overflow_error>([&] { sirkit::min_cost_b_flow(empty_balance, wide); });
    const std::array large_cost{Edge{0, 1, 0, 1, kMax}};
    const std::array small_cost{Edge{0, 1, 0, 1, kMin}};
    must_throw<std::overflow_error>([&] { sirkit::min_cost_b_flow(empty_balance, large_cost); });
    must_throw<std::overflow_error>([&] { sirkit::min_cost_b_flow(empty_balance, small_cost); });
    const std::array to_positive{Edge{0, 1, 1, 1, 0}};
    const std::array to_negative{Edge{0, 1, -1, -1, 0}};
    const std::array<Value, 2> positive_overflow{0, kMax}, negative_overflow{0, kMin}, lower_overflow{kMin, 0};
    must_throw<std::overflow_error>([&] { sirkit::min_cost_b_flow(positive_overflow, to_positive); });
    must_throw<std::overflow_error>([&] { sirkit::min_cost_b_flow(negative_overflow, to_negative); });
    must_throw<std::overflow_error>([&] { sirkit::min_cost_b_flow(lower_overflow, to_positive); });
}

void check_random()
{
    std::mt19937 random(20261128);
    for (int trial = 0; trial < 1800; ++trial)
    {
        const int n = 1 + static_cast<int>(random() % 5), m = static_cast<int>(random() % 8);
        std::vector<Value> supply(n);
        std::vector<Edge> edges;
        for (int i = 0; i < m; ++i)
        {
            const auto u = static_cast<int>(random() % n), v = static_cast<int>(random() % n);
            const auto lower = static_cast<Value>(random() % 5) - 2;
            const auto width = static_cast<Value>(random() % 4);
            const auto sample = lower + static_cast<Value>(random() % (width + 1));
            edges.push_back({u, v, lower, lower + width, static_cast<Value>(random() % 17) - 8});
            supply[u] += sample;
            supply[v] -= sample;
        }
        if (trial % 3 == 0)
        {
            // 合計を保ったまま片方の供給を変え、容量不足を含めて照合する。
            ++supply[random() % n];
            --supply[random() % n];
        }
        if (trial % 3 == 1)
        {
            ++supply[random() % n];
        }
        compare(supply, edges, brute_cost(supply, edges));
    }
}
}

int main()
{
    compare({}, {}, 0);
    compare({0}, std::array{Edge{0, 0, -3, 2, -4}, Edge{0, 0, -3, 2, 4}}, -20);
    compare({1, -1}, std::array{Edge{0, 1, 0, 2, 1}, Edge{1, 0, 0, 1, -3}}, -1);
    compare({1, -1}, std::array{Edge{1, 0, 0, 1, 5}}, std::nullopt);
    check_exceptions();
    check_random();
}
