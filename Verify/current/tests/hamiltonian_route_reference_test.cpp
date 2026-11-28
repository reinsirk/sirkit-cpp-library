// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Hamiltonian_Route.cpp"

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

using Weight = std::int64_t;
using OptionalWeight = std::optional<Weight>;
using Route = sirkit::HamiltonianRoute<Weight>;

template <class CostAt>
static void check(std::size_t size, std::size_t start, const CostAt &cost_at, std::optional<std::size_t> end, bool cycle)
{
    std::vector<std::size_t> order;
    for (std::size_t i = 0; i < size; ++i)
    {
        if (i != start)
        {
            order.push_back(i);
        }
    }
    const auto score = [&](const std::vector<std::size_t> &path) -> OptionalWeight
    {
        Weight total = 0;
        for (std::size_t i = 1; i < path.size(); ++i)
        {
            if (path[i - 1] == path[i])
            {
                assert(path.size() == 2 && size == 1);
                continue;
            }
            const auto edge = OptionalWeight{cost_at(path[i - 1], path[i])};
            if (!edge)
            {
                return std::nullopt;
            }
            total += *edge;
        }
        return total;
    };
    OptionalWeight expected;
    do
    {
        std::vector<std::size_t> path{start};
        path.insert(path.end(), order.begin(), order.end());
        if (cycle)
        {
            path.push_back(start);
        }
        if (end && path.back() != *end)
        {
            continue;
        }
        const auto candidate = score(path);
        if (candidate && (!expected || *candidate < *expected))
        {
            expected = candidate;
        }
    } while (std::ranges::next_permutation(order).found);

    const std::optional<Route> actual = cycle ? sirkit::shortest_hamiltonian_cycle(size, start, cost_at)
                                              : sirkit::shortest_hamiltonian_path(size, start, cost_at, end);
    assert(actual.has_value() == expected.has_value());
    if (actual)
    {
        const auto &path = actual->vertices;
        assert(path.size() == size + cycle && path.front() == start);
        assert(!end || path.back() == *end);
        assert(!cycle || path.back() == start);
        assert(actual->cost == *expected && score(path) == expected);
        std::vector<bool> visited(size);
        for (std::size_t i = 0; i < size; ++i)
        {
            assert(path[i] < size && !visited[path[i]]);
            visited[path[i]] = true;
        }
    }
}

int main()
{
    const auto dense = [](std::size_t u, std::size_t v)
    {
        assert(u != v);
        return static_cast<Weight>(u * 547 + v * 843) - 5097;
    };
    const auto empty = [](std::size_t, std::size_t) -> OptionalWeight
    {
        return std::nullopt;
    };
    check(1, 0, empty, 0, false);
    check(1, 0, dense, std::nullopt, true);
    check(3, 2, dense, 2, false);
    check(2, 1, empty, std::nullopt, true);
    check(3, 1, dense, std::nullopt, false);

    const auto missing_close = [](std::size_t u, std::size_t v) -> OptionalWeight
    {
        if (u + 1 == v)
        {
            return -13;
        }
        return std::nullopt;
    };
    check(4, 0, missing_close, 3, false);
    check(4, 0, missing_close, std::nullopt, true);

    std::mt19937 rng(379525);
    for (int trial = 0; trial < 260; ++trial)
    {
        const std::size_t n = 2 + rng() % 7;
        const auto start = static_cast<std::size_t>(rng() % n);
        std::vector<OptionalWeight> graph(n * n);
        for (auto &edge : graph)
        {
            if (rng() % 4 != 0)
            {
                edge = static_cast<int>(rng() % 200) - 100;
            }
        }
        const auto weight = [&](std::size_t u, std::size_t v)
        {
            assert(u != v);
            return graph[u * n + v];
        };
        check(n, start, weight, std::nullopt, true);
        check(n, start, weight, std::nullopt, false);
        check(n, start, weight, rng() % n, false);
    }

    bool caught = false;
    try
    {
        (void)sirkit::shortest_hamiltonian_cycle(0, 0, dense);
    }
    catch (const std::out_of_range &)
    {
        caught = true;
    }
    assert(caught);
    caught = false;
    try
    {
        (void)sirkit::shortest_hamiltonian_path(2, 0, dense, 2);
    }
    catch (const std::out_of_range &)
    {
        caught = true;
    }
    assert(caught);
    constexpr auto kBits = static_cast<std::size_t>(std::numeric_limits<std::size_t>::digits);
    for (auto size : {kBits, kBits + 1})
    {
        caught = false;
        try
        {
            (void)sirkit::shortest_hamiltonian_cycle(size, 0, dense);
        }
        catch (const std::length_error &)
        {
            caught = true;
        }
        assert(caught);
    }
}
