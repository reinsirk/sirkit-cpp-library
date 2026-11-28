// SPDX-License-Identifier: CC0-1.0
#include "../../../Graph/Difference_Constraints.cpp"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <random>
#include <stdexcept>
#include <vector>

namespace
{
using Constraint = sirkit::DifferenceConstraint;
using Value = std::int64_t;

void compare(int n, const std::vector<Constraint> &constraints, int anchor)
{
    constexpr Value kInfinity = 10000000000000000;
    std::vector<std::vector<Value>> closure(n, std::vector<Value>(n, kInfinity));
    for (int v = 0; v < n; ++v)
    {
        closure[v][v] = 0;
    }
    for (const auto &[u, v, weight] : constraints)
    {
        closure[u][v] = std::min(closure[u][v], weight);
    }
    for (int k = 0; k < n; ++k)
    {
        for (int i = 0; i < n; ++i)
        {
            for (int j = 0; j < n; ++j)
            {
                if (closure[i][k] != kInfinity && closure[k][j] != kInfinity)
                {
                    closure[i][j] = std::min(closure[i][j], closure[i][k] + closure[k][j]);
                }
            }
        }
    }
    bool possible = true;
    for (int v = 0; v < n; ++v)
    {
        possible = possible && closure[v][v] >= 0;
    }

    const auto answer = sirkit::solve_difference_constraints(n, constraints, anchor);
    assert(answer.feasible == possible);
    if (!possible)
    {
        assert(answer.values.empty() && answer.bounds.empty());
        return;
    }
    assert(answer.values.size() == static_cast<std::size_t>(n));
    assert(answer.bounds.size() == static_cast<std::size_t>(n));
    assert(answer.values[anchor] == 0);
    for (const auto &[u, v, weight] : constraints)
    {
        assert(answer.values[v] - answer.values[u] <= weight);
    }
    for (int v = 0; v < n; ++v)
    {
        const auto &[lower, upper] = answer.bounds[v];
        const auto to = closure[anchor][v], from = closure[v][anchor];
        assert(upper == (to == kInfinity ? std::nullopt : std::optional<Value>{to}));
        assert(lower == (from == kInfinity ? std::nullopt : std::optional<Value>{-from}));
        assert(!lower || *lower <= answer.values[v]);
        assert(!upper || answer.values[v] <= *upper);
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
    compare(1, {}, 0);
    compare(2, {{0, 1, 4}, {1, 0, -2}}, 0);
    compare(5, {{0, 1, 3}, {2, 0, -4}, {0, 1, 7}, {2, 2, 0}, {3, 4, 1}}, 0);
    compare(4, {{2, 3, 1}, {3, 2, -2}}, 0);
    compare(4, {{3, 3, -1}}, 1);
    compare(7, {{1, 0, -1000000000}, {2, 1, -1000000000}, {3, 2, -1000000000}, {4, 3, -1000000000}}, 4);
    compare(2, {{0, 1, 1000000000}, {1, 0, 1000000000}}, 0);

    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::solve_difference_constraints(0, {});
                                    });
    expect_error<std::length_error>([]
                                    {
                                        (void)sirkit::solve_difference_constraints(std::numeric_limits<int>::max(), {});
                                    });
    for (const int v : {-1, 2})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            (void)sirkit::solve_difference_constraints(2, {}, v);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array invalid{Constraint{v, 0, 0}};
                                            (void)sirkit::solve_difference_constraints(2, invalid);
                                        });
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array invalid{Constraint{0, v, 0}};
                                            (void)sirkit::solve_difference_constraints(2, invalid);
                                        });
    }
    for (const Value weight : {-1000000001LL, 1000000001LL})
    {
        expect_error<std::out_of_range>([&]
                                        {
                                            const std::array invalid{Constraint{0, 1, weight}};
                                            (void)sirkit::solve_difference_constraints(2, invalid);
                                        });
    }

    const std::array limited{Constraint{0, 1, 4}, Constraint{1, 0, -2}};
    const auto custom = sirkit::solve_difference_constraints(2, limited, 0, 50);
    assert(custom.feasible && custom.bounds[1].lower == 2 && custom.bounds[1].upper == 4);

    std::mt19937 random(2117732);
    for (int trial = 0; trial < 680; ++trial)
    {
        const int n = static_cast<int>(random() % 8) + 1;
        const int m = static_cast<int>(random() % 22);
        std::vector<Constraint> constraints;
        for (int i = 0; i < m; ++i)
        {
            constraints.push_back({static_cast<int>(random() % n), static_cast<int>(random() % n),
                                   static_cast<int>(random() % 17) - 8});
        }
        compare(n, constraints, static_cast<int>(random() % n));
    }

    constexpr int kSize = 2001;
    std::vector<Constraint> deep;
    for (int v = 1; v < kSize; ++v)
    {
        deep.push_back({v, v - 1, 1000000000});
        deep.push_back({v - 1, v, -1000000000});
    }
    const auto answer = sirkit::solve_difference_constraints(kSize, deep);
    assert(answer.feasible);
    for (int v = 0; v < kSize; ++v)
    {
        const Value target = -static_cast<Value>(v) * 1000000000;
        assert(answer.values[v] == target);
        assert(answer.bounds[v].lower == target && answer.bounds[v].upper == target);
    }
}
