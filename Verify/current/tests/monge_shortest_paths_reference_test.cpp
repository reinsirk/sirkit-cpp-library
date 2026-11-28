// SPDX-License-Identifier: CC0-1.0
#include "../../../Algorithm/Monge_Shortest_Paths.cpp"

#include <cassert>
#include <compare>
#include <cstdint>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <vector>

namespace
{
using Value = std::int64_t;

template<class T, class Cost>
sirkit::MongeShortestPathResult<T> brute(int n, T initial, Cost cost)
{
    sirkit::MongeShortestPathResult<T> answer;
    answer.distances.push_back(initial);
    answer.predecessors.push_back(-1);
    for (int to = 1; to < n; ++to)
    {
        int predecessor = 0;
        T best = initial + cost(0, to);
        for (int from = 1; from < to; ++from)
        {
            const T value = answer.distances[from] + cost(from, to);
            if (value < best)
            {
                best = value;
                predecessor = from;
            }
        }
        answer.distances.push_back(best);
        answer.predecessors.push_back(predecessor);
    }
    return answer;
}

struct Score
{
    Value cost;
    int steps;

    Score(Value c, int s) : cost(c), steps(s)
    {
    }

    auto operator<=>(const Score &) const = default;

    Score operator+(const Score &other) const
    {
        return {cost + other.cost, steps + other.steps};
    }
};

void check_initial_and_ties()
{
    const auto empty = sirkit::monge_shortest_paths(0, [](int, int) -> Value {
        assert(false);
        return 0;
    });
    assert(empty.distances.empty() && empty.predecessors.empty());

    const auto single = sirkit::monge_shortest_paths(1, [](int, int) -> Value {
        assert(false);
        return 0;
    }, -257);
    static_assert(std::is_same_v<typename decltype(single.distances)::value_type, Value>);
    assert(single.distances == std::vector<Value>{-257} && single.predecessors == std::vector<int>{-1});

    const auto ties = sirkit::monge_shortest_paths(35, [](int, int) -> Value { return 0; });
    for (int v = 1; v < 35; ++v)
    {
        assert(ties.distances[v] == 0 && ties.predecessors[v] == 0);
    }

    const auto callback = [](int from, int to) {
        const Value d = to - from;
        return Score(d * d + 2, 1);
    };
    const Score initial(12, 7);
    const auto lex = sirkit::monge_shortest_paths(70, callback, initial);
    const auto reference = brute(70, initial, callback);
    assert(lex.distances == reference.distances && lex.predecessors == reference.predecessors);

#ifndef SKIP_RUNTIME_CHECKS
    bool rejected = false;
    try
    {
        static_cast<void>(sirkit::monge_shortest_paths(-1, [](int, int) { return 0; }));
    }
    catch (const std::invalid_argument &)
    {
        rejected = true;
    }
    assert(rejected);
#endif
}

void compare_random_monge_matrices()
{
    std::mt19937_64 rng(128153);
    for (int trial = 0; trial < 1470; ++trial)
    {
        const int n = 2 + rng() % 105;
        std::vector<Value> rows(n), columns(n);
        for (int i = 0; i < n; ++i)
        {
            rows[i] = static_cast<Value>(rng() % 123) - 61;
            columns[i] = static_cast<Value>(rng() % 123) - 61;
        }
        std::vector<std::vector<Value>> prefix(n, std::vector<Value>(n));
        for (int row = 1; row < n; ++row)
        {
            for (int col = 1; col < n; ++col)
            {
                prefix[row][col] = static_cast<Value>(rng() % 4) + prefix[row - 1][col] +
                                   prefix[row][col - 1] - prefix[row - 1][col - 1];
            }
        }
        const auto cost = [&](int from, int to) {
            assert(0 <= from && from < to && to < n);
            return rows[to] + columns[from] - prefix[to][from];
        };
        const Value initial = static_cast<Value>(rng() % 50) - 25;
        const auto answer = sirkit::monge_shortest_paths(n, cost, initial);
        const auto expected = brute(n, initial, cost);
        assert(answer.distances == expected.distances);
        assert(answer.predecessors == expected.predecessors);
    }
}

void validate_linear_number_of_calls()
{
    for (const int n : {2, 3, 130, 1023, 1024, 1025, 400001})
    {
        int calls = 0;
        const auto result = sirkit::monge_shortest_paths(n, [&](int from, int to) {
            assert(0 <= from && from < to && to < n);
            ++calls;
            const Value length = to - from;
            return length * length + 3;
        });
        assert(calls <= 16 * n);
        for (int v = 1; v < n; ++v)
        {
            assert(result.distances[v] == 7 * (v / 2) + 4 * (v % 2));
            assert(result.predecessors[v] == (v == 1 ? 0 : v - 2));
        }
    }
}
}

int main()
{
    check_initial_and_ties();
    compare_random_monge_matrices();
    validate_linear_number_of_calls();
}
